#include <chrono>
#include <iterator> // for std::distance()
#include <string>

#include <boost/lexical_cast.hpp>

#include <gtest/gtest.h>

#include <mac_time_tracker/address.hpp>
#include <mac_time_tracker/period_map.hpp>
#include <mac_time_tracker/time.hpp>

namespace mtt = mac_time_tracker;

TEST(PeriodMap, generalUse) {
  const mtt::Time base_time = mtt::Time::now();
  const std::chrono::seconds duration(1);
  const mtt::Address addr[] = {mtt::Address::fromStr("00:11:22:33:44:55"),
                               mtt::Address::fromStr("66:77:88:99:AA:BB")};
  mtt::PeriodMap period_map;

  // make an instance of PeriodMap
  for (int i_time = 0; i_time < 100; ++i_time) {
    const mtt::Time start[] = {base_time - i_time * duration, base_time + i_time * duration};
    const mtt::Time end[] = {start[0] + duration, start[1] + duration};
    for (int i_info = 0; i_info < 10; ++i_info) {
      const std::string cat = "Category" + boost::lexical_cast<std::string>(i_info),
                        desc = "Description" + boost::lexical_cast<std::string>(i_info);
      period_map.insert({{start[0], end[0]}, {addr[0], cat, desc}});
      period_map.insert({{start[1], end[1]}, {addr[0], cat, desc}});
      period_map.insert({{start[1], end[1]}, {addr[1], cat, desc}});
    }
  }

  // check data in the instance
  ASSERT_EQ(3000, period_map.size());
  for (const mtt::PeriodMap::value_type entry : period_map) {
    const mtt::PeriodMap::Period &period = entry.first;
    const mtt::PeriodMap::Info &info = entry.second;
    // key value
    ASSERT_EQ(period.first + duration, period.second);
    // mapped value
    const std::pair<mtt::PeriodMap::const_iterator, mtt::PeriodMap::const_iterator> range =
        period_map.equal_range(period);
    const std::size_t count = std::distance(range.first, range.second);
    if (period.first < base_time) {
      ASSERT_EQ(10, count);
      ASSERT_TRUE(info.address == addr[0]);
    } else if (period.first == base_time) {
      ASSERT_EQ(30, count);
      ASSERT_TRUE(info.address == addr[0] || info.address == addr[1]);
    } else {
      ASSERT_EQ(20, count);
      ASSERT_TRUE(info.address == addr[0] || info.address == addr[1]);
    }
  }
}

TEST(PeriodMap, filled) {
  namespace sc = std::chrono;

  const mtt::Time base_time = mtt::Time::now();
  const mtt::PeriodMap::Info info[] = {
      {mtt::Address::fromStr("00:11:22:33:44:55"), "Category0", "Description0"},
      {mtt::Address::fromStr("66:77:88:99:AA:BB"), "Category1", "Description1"}};
  mtt::PeriodMap period_map;

  // make an instance of PeriodMap
  period_map.insert({{base_time, base_time + sc::minutes(10)}, info[0]});
  // no gap here (touches)
  period_map.insert({{base_time + sc::minutes(10), base_time + sc::minutes(20)}, info[0]});
  // 10 mins gap, which should be filled
  period_map.insert({{base_time + sc::minutes(30), base_time + sc::minutes(40)}, info[0]});
  // 60 mins gap, which should be filled
  period_map.insert({{base_time + sc::minutes(100), base_time + sc::minutes(110)}, info[0]});
  // 100 mins gap
  period_map.insert({{base_time + sc::minutes(210), base_time + sc::minutes(220)}, info[0]});
  // 10 mins gap but info is different
  period_map.insert({{base_time + sc::minutes(230), base_time + sc::minutes(240)}, info[1]});
  // no gap (overlaps)
  period_map.insert({{base_time + sc::minutes(235), base_time + sc::minutes(245)}, info[1]});
  ASSERT_EQ(7, period_map.size());

  // fill gaps equal to or less than 60 mins
  const mtt::PeriodMap filled_map =
      period_map.filled(sc::hours(1), /* suffix for filling entry's description = */ "-filled");
  ASSERT_EQ(9, filled_map.size());

  // check what was filled
  const mtt::PeriodMap::const_iterator filled_entry[] = {
      filled_map.find({base_time + sc::minutes(20), base_time + sc::minutes(30)}),
      filled_map.find({base_time + sc::minutes(40), base_time + sc::minutes(100)})};
  ASSERT_NE(filled_map.end(), filled_entry[0]);
  ASSERT_EQ(info[0].address, filled_entry[0]->second.address);
  ASSERT_EQ(info[0].category, filled_entry[0]->second.category);
  ASSERT_EQ(info[0].description + "-filled", filled_entry[0]->second.description);
  ASSERT_NE(filled_map.end(), filled_entry[1]);
  ASSERT_EQ(info[0].address, filled_entry[1]->second.address);
  ASSERT_EQ(info[0].category, filled_entry[1]->second.category);
  ASSERT_EQ(info[0].description + "-filled", filled_entry[1]->second.description);
}
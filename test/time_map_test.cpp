#include <chrono>
#include <iterator> // for std::distance()
#include <string>

#include <boost/lexical_cast.hpp>

#include <gtest/gtest.h>

#include <mac_time_tracker/address.hpp>
#include <mac_time_tracker/time.hpp>
#include <mac_time_tracker/time_map.hpp>

namespace mtt = mac_time_tracker;

TEST(TimeMap, generalUse) {
  const mtt::Time base_time = mtt::Time::now();
  const mtt::Address addr[] = {mtt::Address::fromStr("00:11:22:33:44:55"),
                               mtt::Address::fromStr("66:77:88:99:AA:BB")};
  mtt::TimeMap time_map;
  // make an instance of TimeMap
  for (int i_time = 0; i_time < 100; ++i_time) {
    const mtt::Time time[] = {base_time - i_time * std::chrono::seconds(1),
                              base_time + i_time * std::chrono::seconds(1)};
    for (int i_info = 0; i_info < 10; ++i_info) {
      const std::string cat = "Category" + boost::lexical_cast<std::string>(i_info),
                        desc = "Description" + boost::lexical_cast<std::string>(i_info);
      time_map.insert({time[0], {addr[0], cat, desc}});
      time_map.insert({time[1], {addr[0], cat, desc}});
      time_map.insert({time[1], {addr[1], cat, desc}});
    }
  }
  // check data in the instance
  ASSERT_EQ(3000, time_map.size());
  for (const mtt::TimeMap::value_type entry : time_map) {
    const std::pair<mtt::TimeMap::const_iterator, mtt::TimeMap::const_iterator> range =
        time_map.equal_range(entry.first);
    const std::size_t count = std::distance(range.first, range.second);
    if (entry.first < base_time) {
      ASSERT_EQ(10, count);
      ASSERT_TRUE(entry.second.address == addr[0]);
    } else if (entry.first == base_time) {
      ASSERT_EQ(30, count);
      ASSERT_TRUE(entry.second.address == addr[0] || entry.second.address == addr[1]);
    } else {
      ASSERT_EQ(20, count);
      ASSERT_TRUE(entry.second.address == addr[0] || entry.second.address == addr[1]);
    }
  }
}
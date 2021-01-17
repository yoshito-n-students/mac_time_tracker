#include <chrono>
#include <string>
#include <vector>

#include <boost/lexical_cast.hpp>

#include <gtest/gtest.h>

#include <mac_time_tracker/address.hpp>
#include <mac_time_tracker/set.hpp>
#include <mac_time_tracker/time.hpp>
#include <mac_time_tracker/time_bimap.hpp>

namespace mtt = mac_time_tracker;

TEST(TimeBimap, generalUse) {
  const mtt::Time base_time = mtt::Time::now();
  mtt::TimeBimap time_map;
  // make an instance of TimeBimap
  {
    const mtt::Address addr[] = {mtt::Address::fromStr("00:11:22:33:44:55"),
                                 mtt::Address::fromStr("66:77:88:99:AA:BB")};
    for (int i_time = 0; i_time < 100; ++i_time) {
      const mtt::Time time[] = {base_time - i_time * std::chrono::seconds(1),
                                base_time + i_time * std::chrono::seconds(1)};
      for (int i_cat = 0; i_cat < 10; ++i_cat) {
        const std::string cat = "Category" + boost::lexical_cast<std::string>(i_cat);
        ASSERT_TRUE(time_map.insert({time[0], cat, addr[0]}).second);
        ASSERT_TRUE(time_map.insert({time[1], cat, addr[0]}).second);
        ASSERT_TRUE(time_map.insert({time[1], cat, addr[1]}).second);
      }
    }
  }
  // check data in the instance
  {
    ASSERT_EQ(3000, time_map.size());
    const std::vector<mtt::Time> times = time_map.uniqueTimes();
    ASSERT_EQ(199, times.size());
    const std::vector<std::string> cats = time_map.uniqueCategories();
    ASSERT_EQ(10, cats.size());
    for (const mtt::Time &t : times) {
      for (const std::string &c : cats) {
        const mtt::Set a = time_map.addressesAt(t, c);
        if (t < base_time) {
          ASSERT_EQ(1, a.size());
        } else {
          ASSERT_EQ(2, a.size());
        }
      }
    }
  }
}
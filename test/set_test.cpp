#include <gtest/gtest.h>

#include <mac_time_tracker/address.hpp>
#include <mac_time_tracker/set.hpp>

namespace mtt = mac_time_tracker;

TEST(Set, generalUse) {
  const mtt::Address addr[] = {
      mtt::Address::fromStr("00:11:22:33:44:55"), mtt::Address::fromStr("66:77:88:99:AA:BB"),
      mtt::Address::fromStr("CC:DD:EE:FF:00:11"), mtt::Address::fromStr("22:33:44:55:66:77")};
  // Construction with a initializer list
  mtt::Set set;
  set = {addr[0], addr[1], addr[2]};
  ASSERT_EQ(3, set.size());
  set = {addr[0], addr[1], addr[2], addr[1], addr[0]};
  ASSERT_EQ(3, set.size());
  // Insertion: cannot insert existing values
  ASSERT_FALSE(set.insert(addr[0]).second);
  ASSERT_FALSE(set.insert(addr[1]).second);
  ASSERT_FALSE(set.insert(addr[2]).second);
  // Insertion: can insert a new value only once
  ASSERT_EQ(0, set.count(addr[3]));
  ASSERT_TRUE(set.insert(addr[3]).second);
  ASSERT_EQ(1, set.count(addr[3]));
  ASSERT_FALSE(set.insert(addr[3]).second);
  ASSERT_EQ(1, set.count(addr[3]));
  // Removal
  ASSERT_EQ(1, set.count(addr[0]));
  ASSERT_EQ(1, set.erase(addr[0]));
  ASSERT_EQ(set.end(), set.find(addr[0]));
  ASSERT_EQ(0, set.erase(addr[0]));
}
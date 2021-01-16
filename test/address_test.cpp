#include <stdexcept>

#include <gtest/gtest.h>

#include <mac_time_tracker/address.hpp>

namespace mtt = mac_time_tracker;

TEST(Address, fromStr) {
  // Invalid constructions
  EXPECT_THROW(mtt::Address::fromStr(""), std::runtime_error);
  EXPECT_THROW(mtt::Address::fromStr("192.168.0.1"), std::runtime_error);
  EXPECT_THROW(mtt::Address::fromStr("00:11:22"), std::runtime_error);
  EXPECT_THROW(mtt::Address::fromStr("00:11:22:33:AA:BB:CC:DD"), std::runtime_error);
  EXPECT_THROW(mtt::Address::fromStr("000:11:22:33:AA:BB:CC"), std::runtime_error);
  EXPECT_THROW(mtt::Address::fromStr("0:11:22:33:AA:BB:CC"), std::runtime_error);
  EXPECT_THROW(mtt::Address::fromStr("BB:CC:DD:EE:FF:GG"), std::runtime_error);
  EXPECT_THROW(mtt::Address::fromStr("00: 11:22:33:44:55"), std::runtime_error);
  EXPECT_THROW(mtt::Address::fromStr("AA-00-BB-11"), std::runtime_error);
  EXPECT_THROW(mtt::Address::fromStr("AA-00-BB-11-aa-22-dd"), std::runtime_error);
  EXPECT_THROW(mtt::Address::fromStr("gg-ff-ee-dd-cc-bb"), std::runtime_error);
  EXPECT_THROW(mtt::Address::fromStr("00:AA-11:bb- 22:Cc"), std::runtime_error);
  // Valid constructions
  EXPECT_NO_THROW(mtt::Address::fromStr("00:11:22:33:44:55"));
  EXPECT_NO_THROW(mtt::Address::fromStr("00:aa:11:BB:44:cc"));
  EXPECT_NO_THROW(mtt::Address::fromStr("aA-11-Bb-22-cC-33"));
  EXPECT_NO_THROW(mtt::Address::fromStr("00:AA-11:bb-22:Cc"));
  EXPECT_NO_THROW(mtt::Address::fromStr(" 00:AA-11:bb-22:Cc"));
  EXPECT_NO_THROW(mtt::Address::fromStr("00:AA-11:bb-22:Cc "));
  // Same addresses
  EXPECT_EQ(mtt::Address::fromStr("00:11:22:33:44:55"), mtt::Address::fromStr("00-11-22-33-44-55"));
  EXPECT_EQ(mtt::Address::fromStr("00:11-22:33-44:55"), mtt::Address::fromStr("00-11:22-33:44-55"));
  EXPECT_EQ(mtt::Address::fromStr("a0:1B:c2:3D:e4:5D"), mtt::Address::fromStr("A0-1b-C2-3d-E4-5d"));
  // Different addresses
  EXPECT_NE(mtt::Address::fromStr("00:11:22:33:44:55"), mtt::Address::fromStr("11-22-33-44-55-66"));
  EXPECT_NE(mtt::Address::fromStr("a0:1B:c2:3D:e4:5D"), mtt::Address::fromStr("0a:B1:2c:D3:4e:D5"));
}

TEST(Address, toStr) {
  EXPECT_STREQ(mtt::Address::fromStr("00:aa:11:BB:22:cc").toStr().c_str(), "00:AA:11:BB:22:CC");
  EXPECT_STREQ(mtt::Address::fromStr("00:aA:11:bB:22:Cc").toStr('-').c_str(), "00-AA-11-BB-22-CC");
}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
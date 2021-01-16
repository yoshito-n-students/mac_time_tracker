#include <stdexcept>

#include <gtest/gtest.h>

#include <mac_time_tracker/address.hpp>

namespace mtt = mac_time_tracker;

TEST(Address, constructor) {
  // Valid constructors
  const mtt::Address a = {};
  const mtt::Address b = {0x01, 0xAB, 0x23, 0xCD, 0x45, 0xEF};
  EXPECT_EQ(0x01, b[0]);
  EXPECT_EQ(0xAB, b[1]);
  EXPECT_EQ(0x23, b[2]);
  EXPECT_EQ(0xCD, b[3]);
  EXPECT_EQ(0x45, b[4]);
  EXPECT_EQ(0xEF, b[5]);
  const mtt::Address c = b;
  EXPECT_EQ(b[0], c[0]);
  EXPECT_EQ(b[1], c[1]);
  EXPECT_EQ(b[2], c[2]);
  EXPECT_EQ(b[3], c[3]);
  EXPECT_EQ(b[4], c[4]);
  EXPECT_EQ(b[5], c[5]);
  // Invalid constructors
  // const mtt::Address d = {0x01, 0xAB, 0x23, 0xCD, 0x45};
  // const mtt::Address e = {0x01, 0xAB, 0x23, 0xCD, 0x45, 0xEF, 0x67};
}

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
  const mtt::Address a = {0x00, 0xAA, 0x11, 0xBB, 0x22, 0xCC};
  EXPECT_STREQ(a.toStr().c_str(), "00:AA:11:BB:22:CC");
  EXPECT_STREQ(a.toStr('-').c_str(), "00-AA-11-BB-22-CC");
}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
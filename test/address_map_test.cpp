#include <stdexcept>

#include <gtest/gtest.h>

#include <mac_time_tracker/address_map.hpp>

#include "make_temp_file.hpp"

namespace mtt = mac_time_tracker;

TEST(AddressMap, fromFile) {
  // [OK]
  // - address, category and description are OK
  ASSERT_NO_THROW(mtt::AddressMap::fromFile(makeTempFile("00:11:22:33:44:55, John Doe, Phone")));
  // - empty description is OK
  ASSERT_NO_THROW(mtt::AddressMap::fromFile(makeTempFile("00-11-22-33-44-55, John,")));
  // - multiple lines are OK
  ASSERT_NO_THROW(
      mtt::AddressMap::fromFile(makeTempFile("00:11:22:33:44:55, John, Phone\n"
                                             "66:77:88:99:AA:BB, Jane, Work Phone\n"
                                             "CC:DD:EE:FF:00:11, Jane, Private Phone")));
  // [NG]
  // - missing description is NG (2 patterns)
  ASSERT_THROW(mtt::AddressMap::fromFile(makeTempFile("00:11:22:33:44:55, John Doe")),
               std::runtime_error);
  ASSERT_THROW(mtt::AddressMap::fromFile(makeTempFile("00:11:22:33:44:55, John, Phone\n"
                                                      "66:77:88:99:00:AA, John")),
               std::runtime_error);
  // - ill-formed address is NG
  ASSERT_THROW(mtt::AddressMap::fromFile(makeTempFile("172.16.0.100, John, Phone")),
               std::runtime_error);
  // - non-unique address is NG
  ASSERT_THROW(mtt::AddressMap::fromFile(makeTempFile("00-11-22-33-44-55, John, Phone\n"
                                                      "00:11:22:33:44:55, John, Tablet")),
               std::runtime_error);
  // [Data]
  { // Search by address
    const mtt::AddressMap addr_map =
        mtt::AddressMap::fromFile(makeTempFile("00:11:22:33:44:55, Tom, Phone\n"
                                               "66:77:88:99:AA:BB, Tom,\n"
                                               "CC:DD:EE:FF:00:11, Dick, Tablet\n"
                                               "22:33:44:55:66:77, Harry,\n"
                                               "88:99:AA:BB:CC:DD, Harry, PC"));
    const mtt::Address key = mtt::Address::fromStr("22:33:44:55:66:77");
    ASSERT_EQ(1, addr_map.count(key));
    ASSERT_STREQ("Harry", addr_map.find(key)->second.category.c_str());
    ASSERT_STREQ("", addr_map.find(key)->second.description.c_str());
  }
}
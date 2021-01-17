#include <stdexcept>

#include <gtest/gtest.h>

#include <mac_time_tracker/category_bimap.hpp>

#include "make_temp_file.hpp"

namespace mtt = mac_time_tracker;

TEST(CategoryBimap, fromFile) {
  // [OK]
  // - 1 categoty and 1 pair of address and description is OK
  ASSERT_NO_THROW(mtt::CategoryBimap::fromFile(makeTempFile("John Doe, 00:11:22:33:44:55, Phone")));
  // - empty description is OK
  ASSERT_NO_THROW(mtt::CategoryBimap::fromFile(makeTempFile("John, 00-11-22-33-44-55,")));
  // - multiple pairs of address and description are OK
  ASSERT_NO_THROW(mtt::CategoryBimap::fromFile(
      makeTempFile("John, 00-11-22-33-44-55, Phone, 66:77:88:99:AA:bb, Tablet")));
  // - multiple categories are OK
  ASSERT_NO_THROW(mtt::CategoryBimap::fromFile(
      makeTempFile("John, 00:11:22:33:44:55, Phone\n"
                   "Jane, 66:77:88:99:AA:BB, Work Phone, CC:DD:EE:FF:00:11, Private Phone")));
  // [NG]
  // - missing description is NG (3 patterns)
  ASSERT_THROW(mtt::CategoryBimap::fromFile(makeTempFile("John Doe, 00:11:22:33:44:55")),
               std::runtime_error);
  ASSERT_THROW(mtt::CategoryBimap::fromFile(
                   makeTempFile("John, 00:11:22:33:44:55, Phone, 66:77:88:99:00:AA")),
               std::runtime_error);
  ASSERT_THROW(mtt::CategoryBimap::fromFile(makeTempFile("John, 00:11:22:33:44:55, Phone\n"
                                                         "Jane, 66:77:88:99:AA:BB")),
               std::runtime_error);
  // - ill-formed address is NG
  ASSERT_THROW(mtt::CategoryBimap::fromFile(makeTempFile("John, 172.16.0.100, Phone")),
               std::runtime_error);
  // - non-unique address is NG
  ASSERT_THROW(mtt::CategoryBimap::fromFile(
                   makeTempFile("John, 00-11-22-33-44-55, Phone, 00:11:22:33:44:55, Tablet")),
               std::runtime_error);
  // [Data]
  const mtt::CategoryBimap cat_map = mtt::CategoryBimap::fromFile(
      makeTempFile("Tom  , 00:11:22:33:44:55, Phone , 66:77:88:99:AA:BB,\n"
                   "Dick , CC:DD:EE:FF:00:11, Tablet\n"
                   "Harry, 22:33:44:55:66:77,       , 88:99:AA:BB:CC:DD, PC"));
  using Tags = mtt::CategoryBimap::Tags;
  { // Search by address
    using AddressView = mtt::CategoryBimap::map_by<Tags::Address>::type;
    const AddressView &view = cat_map.by<Tags::Address>();
    const mtt::Address key = mtt::Address::fromStr("22:33:44:55:66:77");
    ASSERT_EQ(1, view.count(key));
    ASSERT_STREQ("Harry", view.at(key).c_str());
    ASSERT_STREQ("", view.info_at(key).c_str());
  }
  { // Search by category
    using CategoryView = mtt::CategoryBimap::map_by<Tags::Category>::type;
    const CategoryView &view = cat_map.by<Tags::Category>();
    ASSERT_EQ(2, view.count("Tom"));
    ASSERT_EQ(1, view.count("Dick"));
    ASSERT_EQ(2, view.count("Harry"));
    ASSERT_EQ(0, view.count("John"));
    const CategoryView::const_iterator it = view.find("Dick");
    ASSERT_NE(view.end(), it);
    ASSERT_EQ(mtt::Address::fromStr("CC:DD:EE:FF:00:11"), it->get<Tags::Address>());
    ASSERT_STREQ("Tablet", it->get<Tags::Description>().c_str());
  }
}
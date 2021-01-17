#ifndef MAC_TIME_TRACKER_CATEGORY_BIMAP_HPP
#define MAC_TIME_TRACKER_CATEGORY_BIMAP_HPP

#include <stdexcept>
#include <string>

#include <boost/algorithm/string/trim.hpp>
#include <boost/bimap/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/lexical_cast.hpp>

#include <mac_time_tracker/address.hpp>
#include <mac_time_tracker/csv.hpp>

namespace mac_time_tracker {

///////////////////////////////////////////////////////////////////
// Bimap of category name and MAC address with info of description
// to represent known addresses

struct CategoryBimapTrait {
  struct CategoryTag {};
  struct AddressTag {};
  struct DescriptionTag {};

  template <class Type, class Tag> using Tagged = boost::bimaps::tagged<Type, Tag>;
  using CategoryCollection = boost::bimaps::multiset_of<Tagged<std::string, CategoryTag>>;
  using AddressCollection = boost::bimaps::set_of<Tagged<Address, AddressTag>>;
  using DescriptionInfo = boost::bimaps::with_info<Tagged<std::string, DescriptionTag>>;

  using Base = boost::bimaps::bimap<CategoryCollection, AddressCollection, DescriptionInfo>;
};

class CategoryBimap : public CategoryBimapTrait::Base {
private:
  using Base = CategoryBimapTrait::Base;

public:
  // Tags
  struct Tags {
    using Category = CategoryBimapTrait::CategoryTag;
    using Address = CategoryBimapTrait::AddressTag;
    using Description = CategoryBimapTrait::DescriptionTag;
  };

public:
  // Constructors
  using Base::Base;
  CategoryBimap(const Base &base) : Base(base) {}
  CategoryBimap(Base &&base) : Base(base) {}

  // Create an instance from a file like
  //     0: <c[0]>, <a[0][0]>, <d[0][0]>, ..., <d[0][i]>, <d[0][i]>
  //   ...
  //     k: <c[k]>, <a[k][0]>, <d[k][0]>, ..., <d[k][j]>, <d[k][j]>
  // where c[n]    -> n-th category,
  //       a[n][m] -> m-th MAC address in n-th category
  //       d[n][m] -> description of a[n][m]
  static CategoryBimap fromFile(const std::string &filename) {
    CategoryBimap map;
    for (const std::vector<std::string> &line : CSV::fromFile(filename)) {
      if (line.size() < 3 || line.size() % 2 == 0) {
        throw std::runtime_error("CategoryBimap::fromFile(): Invalid number of elements (" +
                                 boost::lexical_cast<std::string>(line.size()) +
                                 ") in a line of '" + filename + "'");
      }
      for (std::size_t i = 1; i < line.size(); i += 2) {
        // boost::trim_copy() removes leading and trailing spaces
        map.insert({/* category = */ boost::trim_copy(line[0]),
                    /* addr[i] = */ Address::fromStr(boost::trim_copy(line[i])),
                    /* desc[i] = */ boost::trim_copy(line[i + 1])});
      }
    }
    return map;
  }
};
} // namespace mac_time_tracker

#endif
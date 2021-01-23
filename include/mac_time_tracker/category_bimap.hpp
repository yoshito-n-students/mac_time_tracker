#ifndef MAC_TIME_TRACKER_CATEGORY_BIMAP_HPP
#define MAC_TIME_TRACKER_CATEGORY_BIMAP_HPP

#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/algorithm/string/trim.hpp>
#include <boost/bimap/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/lexical_cast.hpp>

#include <mac_time_tracker/address.hpp>
#include <mac_time_tracker/csv.hpp>
#include <mac_time_tracker/io.hpp>

namespace mac_time_tracker {

///////////////////////////////////////////////////////////////////
// Bimap of category name and MAC address with info of description
// to represent known addresses

struct CategoryBimapTraits {
  struct Tags {
    struct Category {};
    struct Address {};
    struct Description {};
  };

  template <class Type, class Tag> using Tagged = boost::bimaps::tagged<Type, Tag>;
  using CategoryMultiset = boost::bimaps::multiset_of<Tagged<std::string, Tags::Category>>;
  using AddressSet = boost::bimaps::set_of<Tagged<Address, Tags::Address>>;
  using WithDescription = boost::bimaps::with_info<Tagged<std::string, Tags::Description>>;

  using Base = boost::bimaps::bimap<CategoryMultiset, AddressSet, WithDescription>;
};

class CategoryBimap : public CategoryBimapTraits::Base, public Readable<CategoryBimap> {
private:
  using Base = CategoryBimapTraits::Base;

public:
  using Tags = CategoryBimapTraits::Tags;

public:
  // Constructors
  using Base::Base;
  CategoryBimap(const Base &base) : Base(base) {}
  CategoryBimap(Base &&base) : Base(base) {}

  // Create an instance from a CSV like
  //     0: <c[0]>, <a[0][0]>, <d[0][0]>, ..., <d[0][i]>, <d[0][i]>
  //   ...
  //     k: <c[k]>, <a[k][0]>, <d[k][0]>, ..., <d[k][j]>, <d[k][j]>
  // where c[n]    -> n-th category,
  //       a[n][m] -> m-th MAC address in n-th category
  //       d[n][m] -> description of a[n][m]
  static CategoryBimap fromCSV(const CSV &csv) {
    CategoryBimap map;
    for (const std::vector<std::string> &line : csv) {
      if (line.size() < 3 || line.size() % 2 == 0) {
        throw std::runtime_error("CategoryBimap::fromCSV(): Invalid number of elements (" +
                                 boost::lexical_cast<std::string>(line.size()) + ")");
      }
      // boost::trim_copy() removes leading and trailing spaces
      const std::string category = boost::trim_copy(line[0]);
      for (std::size_t i = 1; i < line.size(); i += 2) {
        const Address addr = Address::fromStr(boost::trim_copy(line[i]));
        const std::string desc = boost::trim_copy(line[i + 1]);
        if (!map.insert({category, addr, desc}).second) {
          throw std::runtime_error("CategoryBimap::fromCSV(): Cannot insert an item {'" + category +
                                   "', " + addr.toStr() + ", '" + desc +
                                   "'}. Non-unique MAC address?");
        }
      }
    }
    return map;
  }

private:
  virtual void read(std::istream &is) override {
    CSV csv;
    is >> csv;
    try {
      *this = fromCSV(csv);
    } catch (const std::runtime_error &) {
      is.setstate(std::istream::failbit);
    }
  }
};
} // namespace mac_time_tracker

#endif
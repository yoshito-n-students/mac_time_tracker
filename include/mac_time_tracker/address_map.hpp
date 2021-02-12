#ifndef MAC_TIME_TRACKER_ADDRESS_MAP_HPP
#define MAC_TIME_TRACKER_ADDRESS_MAP_HPP

#include <iostream>
#include <map>
#include <stdexcept>
#include <string>

#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>

#include <mac_time_tracker/address.hpp>
#include <mac_time_tracker/csv.hpp>
#include <mac_time_tracker/io.hpp>

namespace mac_time_tracker {

/////////////////////////////////////////////////////////////////////////////////////////
// Map from MAC address to category (ex. owner's name) and description (ex. device type)
// that is useful to represent known addresses

struct AddressMapTraits {
  struct Info {
    std::string category;
    std::string description;
  };
  using Base = std::map<Address, Info>;
};

class AddressMap : public AddressMapTraits::Base, public Readable<AddressMap> {
private:
  using Base = AddressMapTraits::Base;

public:
  using Info = AddressMapTraits::Info;

public:
  // Constructors
  using Base::Base;
  AddressMap(const Base &base) : Base(base) {}
  AddressMap(Base &&base) : Base(base) {}

  // Create an instance from a CSV, each line is '<address>, <category>, <description>'
  static AddressMap fromCSV(const CSV &csv) {
    AddressMap map;
    for (const std::vector<std::string> &line : csv) {
      if (line.size() != 3) {
        // TODO: print line number
        throw std::runtime_error("AddressMap::fromCSV(): Invalid number of elements (" +
                                 boost::lexical_cast<std::string>(line.size()) + ")");
      }
      // boost::trim_copy() removes leading and trailing spaces
      const Address addr = Address::fromStr(boost::trim_copy(line[0]));
      const std::string category = boost::trim_copy(line[1]), desc = boost::trim_copy(line[2]);
      if (!map.insert({addr, {category, desc}}).second) {
        throw std::runtime_error("AddressMap::fromCSV(): Cannot insert an item {'" + addr.toStr() +
                                 "', " + category + ", '" + desc + "'}. Non-unique MAC address?");
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
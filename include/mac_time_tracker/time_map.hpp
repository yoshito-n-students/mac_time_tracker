#ifndef MAC_TIME_TRACKER_TIME_MAP_HPP
#define MAC_TIME_TRACKER_TIME_MAP_HPP

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <mac_time_tracker/address.hpp>
#include <mac_time_tracker/csv.hpp>
#include <mac_time_tracker/io.hpp>
#include <mac_time_tracker/time.hpp>

namespace mac_time_tracker {

//////////////////////////////////////////////////////////////////
// Map from timestamp to MAC address, category and description
// that is useful to represent time history of address appearance

struct TimeMapTraits {
  struct Info {
    Address address;
    std::string category;
    std::string description;
  };
  using Base = std::multimap<Time, Info>;
};

class TimeMap : public TimeMapTraits::Base, public Writable {
private:
  using Base = TimeMapTraits::Base;

public:
  using Info = TimeMapTraits::Info;

public:
  using Base::Base;
  TimeMap(const Base &base) : Base(base) {}
  TimeMap(Base &&base) : Base(base) {}

  // make a CSV, each line is '<timestamp>, <address>, <category>, <description>'
  CSV toCSV(const std::string &time_fmt = Time::defaultFormat(),
            const char addr_sep = Address::defaultSeparator()) const {
    CSV csv;
    for (const value_type &entry : *this) {
      csv.push_back(std::vector<std::string>{entry.first.toStr(time_fmt),
                                             entry.second.address.toStr(addr_sep),
                                             entry.second.category, entry.second.description});
    }
    return csv;
  }

private:
  virtual void write(std::ostream &os) const override { os << toCSV(); }
};

} // namespace mac_time_tracker

#endif
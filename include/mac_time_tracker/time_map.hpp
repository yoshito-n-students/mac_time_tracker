#ifndef MAC_TIME_TRACKER_TIME_MAP_HPP
#define MAC_TIME_TRACKER_TIME_MAP_HPP

#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
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

  void toHTML(const std::string &filename, const std::string &template_str,
              const std::chrono::seconds &duration) const {
    std::ofstream ofs(filename);
    if (!ofs) {
      throw std::runtime_error("TimeMap::toHTML(): Cannot open '" + filename + "' to write");
    }

    static const std::string keyword = "@DATA_ENTRIES@";
    const std::size_t pos = template_str.find(keyword);
    if (pos == std::string::npos) {
      // no keyword in template
      ofs << template_str;
      return;
    }

    ofs << template_str.substr(0, pos);
    for (const value_type &entry : *this) {
      namespace sc = std::chrono;
      char line[256];
      std::sprintf(
          line, "['%s', new Date(%ld), new Date(%ld)], // %ld seconds from %s\n",
          entry.second.category.c_str(),
          sc::duration_cast<sc::milliseconds>(entry.first.time_since_epoch()).count(),
          sc::duration_cast<sc::milliseconds>((entry.first + duration).time_since_epoch()).count(),
          duration.count(), entry.first.toStr().c_str());
      ofs << line;
    }
    ofs << template_str.substr(pos + keyword.size());
  }

private:
  virtual void write(std::ostream &os) const override { os << toCSV(); }
};

} // namespace mac_time_tracker

#endif
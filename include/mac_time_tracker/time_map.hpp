#ifndef MAC_TIME_TRACKER_TIME_MAP_HPP
#define MAC_TIME_TRACKER_TIME_MAP_HPP

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <mac_time_tracker/address.hpp>
#include <mac_time_tracker/csv.hpp>
#include <mac_time_tracker/io.hpp>
#include <mac_time_tracker/time.hpp>

#include <boost/algorithm/string/replace.hpp>

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

    // replace '@DATE@' to last update date
    std::string str = template_str;
    boost::replace_all(str, "@DATE@", Time::now().toStr());

    // replace '@DATA_ENTRIES@' to data
    {
      std::ostringstream entries_str;
      for (const value_type &entry : *this) {
        namespace sc = std::chrono;
        entries_str << "['" << entry.second.category << "', "
                    << "new Date("
                    << sc::duration_cast<sc::milliseconds>(entry.first.time_since_epoch()).count()
                    << "), "
                    << "new Date("
                    << sc::duration_cast<sc::milliseconds>(
                           (entry.first + duration).time_since_epoch())
                           .count()
                    << ")], "
                    << "// " << duration.count() << " seconds from " << entry.first << std::endl;
      }
      boost::replace_all(str, "@DATA_ENTRIES@", entries_str.str());
    }

    // finally write to the file
    ofs << str;
  }

private:
  virtual void write(std::ostream &os) const override { os << toCSV(); }
};

} // namespace mac_time_tracker

#endif
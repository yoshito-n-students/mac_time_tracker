#ifndef MAC_TIME_TRACKER_PERIOD_MAP_HPP
#define MAC_TIME_TRACKER_PERIOD_MAP_HPP

#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility> // for std::pair<>

#include <mac_time_tracker/address.hpp>
#include <mac_time_tracker/csv.hpp>
#include <mac_time_tracker/io.hpp>
#include <mac_time_tracker/time.hpp>

#include <boost/algorithm/string/replace.hpp>

namespace mac_time_tracker {

//////////////////////////////////////////////////////////////////
// Map from timestamp to MAC address, category and description
// that is useful to represent time history of address appearance

struct PeriodMapTraits {
  using Period = std::pair<Time, Time>;
  struct Info {
    Address address;
    std::string category;
    std::string description;
  };
  using Base = std::multimap<Period, Info>;
};

class PeriodMap : public PeriodMapTraits::Base, public Writable {
private:
  using Base = PeriodMapTraits::Base;

public:
  using Period = PeriodMapTraits::Period;
  using Info = PeriodMapTraits::Info;

public:
  using Base::Base;
  PeriodMap(const Base &base) : Base(base) {}
  PeriodMap(Base &&base) : Base(base) {}

  // returns a copy of this after filling empty slots less than max_fill.
  // when inserting a filling entry, append desc_suffix to the description.
  PeriodMap filled(const Time::duration &max_fill, const std::string &desc_suffix = "*") const {
    PeriodMap ret = *this;
    for (const_iterator entry = begin(); entry != end(); ++entry) {
      // find the next entry with the same address. set end() if nothing.
      const_iterator next = entry;
      while (true) {
        ++next;
        if (next == end() || next->first.first - entry->first.second > max_fill) {
          next = end();
          break;
        } else if (next->second.address == entry->second.address) {
          break;
        }
      }
      // insert a filling entry if possible
      if (next != end() && next->first.first > entry->first.second) {
        ret.insert({{entry->first.second, next->first.first},
                    {entry->second.address, entry->second.category,
                     entry->second.description + desc_suffix}});
      }
    }
    return ret;
  }

  // make a CSV, each line is '<timestamp>, <address>, <category>, <description>'
  CSV toCSV(const std::string &time_fmt = Time::defaultFormat(),
            const char addr_sep = Address::defaultSeparator()) const {
    CSV csv;
    for (const value_type &val : *this) {
      const Period &period = val.first;
      const Info &info = val.second;
      csv.push_back(
          std::vector<std::string>{period.first.toStr(time_fmt), period.second.toStr(time_fmt),
                                   info.address.toStr(addr_sep), info.category, info.description});
    }
    return csv;
  }

  void toHTML(const std::string &filename, const std::string &template_str,
              const std::string &time_fmt = Time::defaultFormat(),
              const char addr_sep = Address::defaultSeparator()) const {
    std::ofstream ofs(filename);
    if (!ofs) {
      throw std::runtime_error("TimeMap::toHTML(): Cannot open '" + filename + "' to write");
    }

    // replace '@DATE@' to last update date
    std::string str = template_str;
    boost::replace_all(str, "@DATE@", Time::now().toStr(time_fmt));

    // replace '@DATA_ENTRIES@' to data
    {
      std::ostringstream entries_str;
      for (const_iterator entry = begin(); entry != end(); ++entry) {
        namespace sc = std::chrono;
        if (entry != begin()) {
          entries_str << "," << std::endl;
        }
        const Period &period = entry->first;
        const Info &info = entry->second;
        entries_str << "['" << info.category << "', "
                    << "'" << info.address.toStr(addr_sep) << " (" << info.description << ")', "
                    << "new Date("
                    << sc::duration_cast<sc::milliseconds>(period.first.time_since_epoch()).count()
                    << "), "
                    << "new Date("
                    << sc::duration_cast<sc::milliseconds>(period.second.time_since_epoch()).count()
                    << ")] "
                    << "/* " << period.first.toStr(time_fmt) << " to "
                    << period.second.toStr(time_fmt) << " */";
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
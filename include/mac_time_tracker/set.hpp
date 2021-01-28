#ifndef MAC_TIME_TRACKER_SET_HPP
#define MAC_TIME_TRACKER_SET_HPP

#include <cstdio>
#include <memory>
#include <set>
#include <stdexcept>

#include <stdio.h> // for popen(), pclose()

#include <mac_time_tracker/address.hpp>

namespace mac_time_tracker {

////////////////////////////////////
// Set of MAC addresses
// to represent results of arp-scan

class Set : public std::set<Address> {
private:
  using Base = std::set<Address>;

public:
  using Base::Base;
  Set(const Base &base) : Base(base) {}
  Set(Base &&base) : Base(base) {}

  static Set fromARPScan(const std::string &options = "--localnet") {
    const std::unique_ptr<FILE, int (*)(FILE *)> fp(
        popen(("arp-scan " + options +
               R"( | grep '\([0-9a-fA-F]\{2\}[-:]\)\{5\}\([0-9a-fA-F]\{2\}\)' --only-matching)")
                  .c_str(),
              "r"),
        pclose);
    if (!fp) {
      throw std::runtime_error("Set::fromARPScan(): popen");
    }

    Set set;
    while (true) {
      char line[256];
      if (!std::fgets(line, 256, fp.get())) {
        break; // end of file
      }
      set.insert(Address::fromStr(line));
    }

    return set;
  }
};
} // namespace mac_time_tracker

#endif
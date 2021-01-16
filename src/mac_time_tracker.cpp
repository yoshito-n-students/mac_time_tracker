#include <chrono>
#include <iostream>
#include <stdexcept>

#include <signal.h>

#include <mac_time_tracker/address.hpp>
#include <mac_time_tracker/category_bimap.hpp>
#include <mac_time_tracker/rate.hpp>
#include <mac_time_tracker/set.hpp>
#include <mac_time_tracker/time.hpp>
#include <mac_time_tracker/time_bimap.hpp>

namespace mtt = mac_time_tracker;

mtt::TimeBimap g_log;
void dumpLogAndExit(int /* sig */) {
  g_log.toCSV().toFile("log.csv");
  std::cout << "A log file successfully made" << std::endl;
  exit(0);
}

int main(int argc, char *argv[]) {
  signal(SIGINT, dumpLogAndExit);

  try {
    std::cout << "Loading known addresses ..." << std::endl;
    const mtt::CategoryBimap known_addrs = mtt::CategoryBimap::fromFile("known_addresses.csv");
    for (const mtt::CategoryBimap::value_type &addr : known_addrs) {
      std::cout << "    " << addr.right.toStr() << " (" << addr.left << " > " << addr.info << ")"
                << std::endl;
    }

    mtt::Rate rate(std::chrono::seconds(30));
    for (int i = 0;; ++i) {
      std::cout << "Scanning present addresses ... (#" << i << ")" << std::endl;
      const mtt::Time stamp = rate.startTime();
      const mtt::Set present_addrs = mtt::Set::fromARPScan();
      for (const mtt::Address &addr : present_addrs) {
        using Tags = mtt::CategoryBimap::Tags;
        const mtt::CategoryBimap::map_by<Tags::Address>::const_iterator it(
            known_addrs.by<Tags::Address>().find(addr));
        if (it != known_addrs.by<Tags::Address>().end()) {
          std::cout << "    " << addr.toStr() << " (" << it->get<Tags::Category>() << " > "
                    << it->get<Tags::Description>() << ")" << std::endl;
          g_log.insert({stamp, it->get<Tags::Category>(), addr});
        } else {
          std::cout << "    " << addr.toStr() << " (Unknown)" << std::endl;
          g_log.insert({stamp, "Unknown", addr});
        }
      }
      rate.sleep();
    }
  } catch (const std::exception &err) {
    std::cerr << err.what() << std::endl;
  }

  return 0;
}
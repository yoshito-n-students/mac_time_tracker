#include <chrono>
#include <iostream>
#include <stdexcept>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>        // for parse_command_line()
#include <boost/program_options/value_semantic.hpp> // for value<>() and bool_swich()
#include <boost/program_options/variables_map.hpp>  // for variables_map, store() and notify()

#include <mac_time_tracker/address.hpp>
#include <mac_time_tracker/category_bimap.hpp>
#include <mac_time_tracker/rate.hpp>
#include <mac_time_tracker/set.hpp>
#include <mac_time_tracker/time.hpp>
#include <mac_time_tracker/time_bimap.hpp>

namespace bpo = boost::program_options;
namespace mtt = mac_time_tracker;

int main(int argc, char *argv[]) {
  // Parse command line args
  std::string known_addr_file, tracked_addr_file;
  double scan_period;
  {
    bpo::options_description arg_desc;
    bool help;
    arg_desc.add_options()
        // key, correspinding variable, description
        ("known-addr-csv", bpo::value(&known_addr_file)->default_value("known_addresses.csv"),
         "path to input .csv file that contains known MAC addresses") //
        ("tracked-addr-csv", bpo::value(&tracked_addr_file)->default_value("tracked_addresses.csv"),
         "path to output .csv file that contains tracked MAC addresses") //
        ("scan-period", bpo::value(&scan_period)->default_value(300.),
         "period of MAC address scan in seconds") //
        ("help,h", bpo::bool_switch(&help), "print help message");
    bpo::variables_map arg_map;
    bpo::store(bpo::parse_command_line(argc, argv, arg_desc), arg_map);
    bpo::notify(arg_map);
    if (help) {
      std::cout << arg_desc << std::endl;
      return 0;
    }
  }

  try {
    // Step 1: Load known addresses
    std::cout << "Loading known addresses from '" << known_addr_file << "' ..." << std::endl;
    const mtt::CategoryBimap known_addrs = mtt::CategoryBimap::fromFile(known_addr_file);
    for (const mtt::CategoryBimap::value_type &addr : known_addrs) {
      std::cout << "    " << addr.right.toStr() << " (" << addr.left << " > " << addr.info << ")"
                << std::endl;
    }

    mtt::TimeBimap tracked_addrs;
    mtt::Rate rate(scan_period * std::chrono::seconds(1));
    for (int i = 0;; ++i) {
      // Step 2: Scan addresses in network
      std::cout << "Scanning present addresses ... (#" << i << ")" << std::endl;
      const mtt::Time stamp = mtt::Time::now();
      const mtt::Set present_addrs = mtt::Set::fromARPScan();
      for (const mtt::Address &addr : present_addrs) {
        using Tags = mtt::CategoryBimap::Tags;
        using AddressView = mtt::CategoryBimap::map_by<Tags::Address>::type;
        const AddressView &addr_view = known_addrs.by<Tags::Address>();
        const AddressView::const_iterator it = addr_view.find(addr);
        if (it != addr_view.end()) {
          std::cout << "    " << addr.toStr() << " (" << it->get<Tags::Category>() << " > "
                    << it->get<Tags::Description>() << ")" << std::endl;
          tracked_addrs.insert({stamp, it->get<Tags::Category>(), addr});
        } else {
          std::cout << "    " << addr.toStr() << " (Unknown)" << std::endl;
          tracked_addrs.insert({stamp, "Unknown", addr});
        }
      }

      // Step 3: Save scan results organized using knowledge
      std::cout << "Saving tracked addresses to '" << tracked_addr_file << "' ..." << std::flush;
      tracked_addrs.toCSV().toFile(tracked_addr_file);
      std::cout << " done" << std::endl;

      // Step 4: Sleep until the next scan
      rate.sleep();
    }
  } catch (const std::exception &err) {
    std::cerr << err.what() << std::endl;
  }

  return 0;
}
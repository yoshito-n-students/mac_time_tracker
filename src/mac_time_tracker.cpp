#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>

#include <boost/lexical_cast.hpp>
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

namespace mtt = mac_time_tracker;

struct Parameters {
  std::string known_addr_file, tracked_addr_file_fmt, unknown_category;
  std::chrono::seconds scan_period;
  std::chrono::minutes track_period;
  bool verbose;

  // Det parameters using command line args.
  // If help is requested via command line, non-empty help_msg is also provided.
  static Parameters fromCommandLine(const int argc, const char *const argv[],
                                    std::string *const help_msg) {
    namespace bpo = boost::program_options;
    Parameters params;
    // define command line options
    bpo::options_description arg_desc("mac_time_tracker");
    unsigned int scan_period, track_period;
    bool help;
    arg_desc.add_options()
        // key, correspinding variable, description
        ("known-addr-csv",
         bpo::value(&params.known_addr_file)->default_value("known_addresses.csv"),
         "path to input .csv file that contains known MAC addresses") //
        ("tracked-addr-csv",
         bpo::value(&params.tracked_addr_file_fmt)
             ->default_value("tracked_addresses_%Y-%m-%d-%H-%M-%S.csv"),
         "path to output .csv file that contains tracked MAC addresses") //
        ("unknown-categoty", bpo::value(&params.unknown_category)->default_value("Unknown"),
         "category name for unknown addresses") //
        ("scan-period", bpo::value(&scan_period)->default_value(300),
         "period of MAC address scan in seconds") //
        ("track-period", bpo::value(&track_period)->default_value(60),
         "period to rotate output .cvs files in minutes")                          //
        ("verbose,v", bpo::bool_switch(&params.verbose), "verbose console output") //
        ("help,h", bpo::bool_switch(&help), "print help message");
    // parse command line args
    bpo::variables_map arg_map;
    bpo::store(bpo::parse_command_line(argc, argv, arg_desc), arg_map);
    bpo::notify(arg_map);
    // return results
    params.scan_period = std::chrono::seconds(scan_period);
    params.track_period = std::chrono::minutes(track_period);
    *help_msg = help ? boost::lexical_cast<std::string>(arg_desc) : std::string("");
    return params;
  }
};

void printKnownAddresses(std::ostream &os, const std::string &filename,
                         const mtt::CategoryBimap &known_addrs) {
  using Tags = mtt::CategoryBimap::Tags;
  using CategoryView = mtt::CategoryBimap::map_by<Tags::Category>::type;
  const CategoryView &view = known_addrs.by<Tags::Category>();
  if (!view.empty()) {
    os << "Known addresses from '" << filename << "'" << std::endl;
    for (const CategoryView::value_type &entry : view) {
      os << "    " << entry.get<Tags::Address>().toStr() << " ('" << entry.get<Tags::Category>()
         << "' > '" << entry.get<Tags::Description>() << "')" << std::endl;
    }
  } else {
    os << "No known addresses from '" << filename << "'" << std::endl;
  }
}

void printTrackedAddresses(std::ostream &os, const mtt::TimeBimap &tracked_addresses,
                           const mtt::Time &stamp) {
  using Tags = mtt::TimeBimap::Tags;
  using TimeView = mtt::TimeBimap::map_by<Tags::Time>::type;
  const TimeView &view = tracked_addresses.by<Tags::Time>();
  const std::pair<TimeView::const_iterator, TimeView::const_iterator> range =
      view.equal_range(stamp);
  if (range.first != range.second) {
    os << "Tracked addresses at " << stamp.toStr() << std::endl;
    for (TimeView::const_iterator it = range.first; it != range.second; ++it) {
      os << "    " << it->get<Tags::Address>().toStr() << " ('" << it->get<Tags::Category>() << "')"
         << std::endl;
    }
  } else {
    os << "No tracked addresses at " << stamp.toStr() << std::endl;
  }
}

int main(int argc, char *argv[]) {
  // Parse command line args
  std::string help_msg;
  const Parameters params = Parameters::fromCommandLine(argc, argv, &help_msg);
  if (!help_msg.empty()) {
    std::cout << help_msg << std::endl;
    return 0;
  }

  // Main loop (never returns)
  while (true) {
    // Step 1: Load known addresses
    mtt::CategoryBimap known_addrs;
    try {
      known_addrs = mtt::CategoryBimap::fromFile(params.known_addr_file);
    } catch (const std::exception &err) {
      std::cerr << err.what() << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    if (params.verbose) {
      printKnownAddresses(std::cout, params.known_addr_file, known_addrs);
    }

    // Constants and storage for this tracking period
    const mtt::Time start_time = mtt::Time::now();               // start time
    const mtt::Time end_time = start_time + params.track_period; // end time
    const std::string tracked_addr_file =
        start_time.toStr(params.tracked_addr_file_fmt); // output filename
    mtt::TimeBimap tracked_addrs;                       // storage

    // Tracking loop
    mtt::Rate rate(params.scan_period);
    for (mtt::Time present_time = mtt::Time::now(); present_time < end_time;
         present_time = mtt::Time::now()) {
      try {
        // Step 2: Scan and track addresses in network by matching them to the known addresses
        const mtt::Set present_addrs = mtt::Set::fromARPScan();
        for (const mtt::Address &addr : present_addrs) {
          using Tags = mtt::CategoryBimap::Tags;
          using AddressView = mtt::CategoryBimap::map_by<Tags::Address>::type;
          const AddressView &view = known_addrs.by<Tags::Address>();
          const AddressView::const_iterator it = view.find(addr);
          if (it != view.end()) {
            tracked_addrs.insert({present_time, it->get<Tags::Category>(), addr});
          } else {
            tracked_addrs.insert({present_time, params.unknown_category, addr});
          }
        }
        if (params.verbose) {
          printTrackedAddresses(std::cout, tracked_addrs, present_time);
        }

        // Step 3: Save scan results
        tracked_addrs.toCSV().toFile(tracked_addr_file);
        if (params.verbose) {
          std::cout << "Tracked addresses was saved to " << tracked_addr_file << std::endl;
        }
      } catch (const std::exception &err) {
        std::cerr << err.what() << std::endl;
      }

      // Step 4: Sleep until the next scan
      rate.sleep();
    }
  }

  return 0;
}
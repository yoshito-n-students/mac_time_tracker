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
#include <mac_time_tracker/address_map.hpp>
#include <mac_time_tracker/rate.hpp>
#include <mac_time_tracker/set.hpp>
#include <mac_time_tracker/time.hpp>
#include <mac_time_tracker/time_map.hpp>

namespace mtt = mac_time_tracker;

struct Parameters {
  std::string known_addr_csv, tracked_addr_csv_fmt;
  std::string tracked_addr_html_in, tracked_addr_html_fmt;
  std::string arp_scan_options;
  std::chrono::seconds scan_period;
  std::chrono::minutes track_period;
  bool verbose;

  // Get parameters from command line args.
  // If help is requested via command line, non-empty help_msg is also provided.
  static Parameters fromCommandLine(const int argc, const char *const argv[],
                                    std::string *const help_msg) {
    namespace bpo = boost::program_options;
    Parameters params;
    // define command line options
    bpo::options_description arg_desc(
        "mac_time_tracker",
        /* line length in help msg = */ bpo::options_description::m_default_line_length,
        /* desc length in help msg = */ bpo::options_description::m_default_line_length * 6 / 10);
    bool help;
    arg_desc.add_options()
        // key, correspinding variable, description
        ("known-addr-csv", bpo::value(&params.known_addr_csv)->default_value("known_addresses.csv"),
         "path to input .csv file that contains known MAC addresses\n"
         "  format: <addr>, <category>, <description>\n"
         "     ex.: 00:11:22:33:44:55, John Doe, PC\n"
         "          66:77:88:99:AA:BB, John Doe, Phone\n"
         "          CC:DD:EE:FF:00:11, Jane Smith, Tablet") //
        ("tracked-addr-csv",
         bpo::value(&params.tracked_addr_csv_fmt)
             ->default_value("tracked_addresses_%Y-%m-%d-%H-%M-%S.csv"),
         "path to output .csv file that contains tracked MAC addresses."
         " will be formatted by std::put_time().") //
        ("tracked-addr-html-in",
         bpo::value(&params.tracked_addr_html_in)->default_value("tracked_addresses.html.in"),
         "path to input .html file that will be used as a template."
         " '@DATA_ENTRIES@' keyword in file will be replaced to tracked addresses."
         " if not given, .html file will not be made.") //
        ("tracked-addr-html",
         bpo::value(&params.tracked_addr_html_fmt)
             ->default_value("tracked_addresses_%Y-%m-%d-%H-%M-%S.html"),
         "path to output .html file. will be formatted by std::put_time().") //
        ("arp-scan-options",
         bpo::value(&params.arp_scan_options)->default_value(mtt::Set::defaultOptions()),
         "options for arp-scan") //
        ("scan-period",
         bpo::value<unsigned int>()->default_value(300)->notifier(
             [&params](const unsigned int val) { params.scan_period = std::chrono::seconds(val); }),
         "period of MAC address scan in seconds") //
        ("track-period",
         bpo::value<unsigned int>()->default_value(60)->notifier([&params](const unsigned int val) {
           params.track_period = std::chrono::minutes(val);
         }),
         "period to rotate output .csv and .html files in minutes")                //
        ("verbose,v", bpo::bool_switch(&params.verbose), "verbose console output") //
        ("help,h", bpo::bool_switch(&help), "print help message");
    // parse command line args
    bpo::variables_map arg_map;
    bpo::store(bpo::parse_command_line(argc, argv, arg_desc), arg_map);
    bpo::notify(arg_map);
    // return results
    *help_msg = help ? boost::lexical_cast<std::string>(arg_desc) : std::string("");
    return params;
  }
};

void printKnownAddresses(std::ostream &os, const std::string &filename,
                         const mtt::AddressMap &known_addrs) {
  if (!known_addrs.empty()) {
    os << "Known addresses from '" << filename << "'" << std::endl;
    for (const mtt::AddressMap::value_type &entry : known_addrs) {
      os << "    " << entry.first << " ('" << entry.second.category << "' > '"
         << entry.second.description << "')" << std::endl;
    }
  } else {
    os << "No known addresses from '" << filename << "'" << std::endl;
  }
}

void printTrackedAddresses(std::ostream &os, const mtt::TimeMap &tracked_addrs,
                           const mtt::Time &stamp) {
  const std::pair<mtt::TimeMap::const_iterator, mtt::TimeMap::const_iterator> range =
      tracked_addrs.equal_range(stamp);
  if (range.first != range.second) {
    os << "Tracked addresses at " << stamp << std::endl;
    for (mtt::TimeMap::const_iterator it = range.first; it != range.second; ++it) {
      os << "    " << it->second.address << " ('" << it->second.category << "' > '"
         << it->second.description << "')" << std::endl;
    }
  } else {
    os << "No tracked addresses at " << stamp << std::endl;
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
  for (int i = 0;; ++i) {
    // Step 1: Load known addresses and a template of tracked addresses .html
    mtt::AddressMap known_addrs;
    std::string tracked_addrs_html_in;
    try {
      known_addrs = mtt::AddressMap::fromFile(params.known_addr_csv);
      if (params.verbose) {
        printKnownAddresses(std::cout, params.known_addr_csv, known_addrs);
      }

      std::ifstream ifs(params.tracked_addr_html_in);
      if (!ifs) {
        throw std::runtime_error("Cannot open '" + params.tracked_addr_html_in + "' to read");
      }
      tracked_addrs_html_in.assign(std::istreambuf_iterator<char>(ifs),
                                   std::istreambuf_iterator<char>());
    } catch (const std::exception &err) {
      std::cerr << err.what() << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }

    // Constants and storage for this tracking period
    const mtt::Time start_time = mtt::Time::now();               // start time
    const mtt::Time end_time = start_time + params.track_period; // end time
    const std::string tracked_addr_csv =
        start_time.toStr(params.tracked_addr_csv_fmt); // output .csv filename
    const std::string tracked_addrs_html =
        start_time.toStr(params.tracked_addr_html_fmt); // output .html filename
    mtt::TimeMap tracked_addrs;                         // storage
    if (params.verbose) {
      std::cout << "Tracking period #" << i << "\n"
                << "     start: " << start_time << "\n"
                << "       end: " << end_time << "\n"
                << "    output: " << tracked_addr_csv << ", " << tracked_addrs_html << std::endl;
    }

    // Tracking loop
    mtt::Rate rate(params.scan_period);
    for (mtt::Time present_time = mtt::Time::now(); present_time < end_time;
         present_time = mtt::Time::now()) {
      try {
        // Step 2: Scan and track addresses in network by matching them to the known addresses
        const mtt::Set present_addrs = mtt::Set::fromARPScan(params.arp_scan_options);
        for (const mtt::Address &addr : present_addrs) {
          const mtt::AddressMap::const_iterator it = known_addrs.find(addr);
          if (it != known_addrs.end()) {
            tracked_addrs.insert(
                {present_time, {addr, it->second.category, it->second.description}});
          }
        }
        if (params.verbose) {
          printTrackedAddresses(std::cout, tracked_addrs, present_time);
        }

        // Step 3: Save scan results
        tracked_addrs.toFile(tracked_addr_csv);
        tracked_addrs.toHTML(tracked_addrs_html, tracked_addrs_html_in, params.scan_period);
      } catch (const std::exception &err) {
        std::cerr << err.what() << std::endl;
      }

      // Step 4: Sleep until the next scan
      rate.sleep();
    }
  }

  return 0;
}
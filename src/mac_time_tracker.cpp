#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <boost/algorithm/string/join.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>        // for parse_command_line()
#include <boost/program_options/value_semantic.hpp> // for value<>() and bool_swich()
#include <boost/program_options/variables_map.hpp>  // for variables_map, store() and notify()

#include <mac_time_tracker/address.hpp>
#include <mac_time_tracker/address_map.hpp>
#include <mac_time_tracker/period_map.hpp>
#include <mac_time_tracker/set.hpp>
#include <mac_time_tracker/time.hpp>

namespace mtt = mac_time_tracker;

////////////////////////
// Command line options

struct Parameters {
  std::string known_addr_csv, tracked_addr_html_in;
  std::vector<std::string> tracked_addr_csv_fmts, tracked_addr_html_fmts;
  std::string arp_scan_options;
  std::chrono::minutes scan_interval, track_interval;
  bool verbose;

  // Get parameters from command line args.
  // If help is requested via command line, non-empty help_msg is also provided.
  static Parameters fromCommandLine(const int argc, const char *const argv[],
                                    std::string *const help_msg) {
    namespace bpo = boost::program_options;
    Parameters params;
    bool help;
    // define command line options
    bpo::options_description arg_desc(
        "mac_time_tracker",
        /* line length in help msg = */ bpo::options_description::m_default_line_length,
        /* desc length in help msg = */ bpo::options_description::m_default_line_length * 6 / 10);
    static const std::string default_tracked_addr_csv_fmt =
                                 "tracked_addresses_%Y-%m-%d-%H-%M-%S.csv",
                             default_tracked_addr_html_fmt =
                                 "tracked_addresses_%Y-%m-%d-%H-%M-%S.html";
    arg_desc.add_options()
        // key, correspinding variable, description
        ("known-addr-csv", bpo::value(&params.known_addr_csv)->default_value("known_addresses.csv"),
         "path to input .csv file that contains known MAC addresses\n"
         "  format: <addr>, <category>, <description>\n"
         "     ex.: 00:11:22:33:44:55, John Doe, PC\n"
         "          66:77:88:99:AA:BB, John Doe, Phone\n"
         "          CC:DD:EE:FF:00:11, Jane Smith, Tablet") //
        ("tracked-addr-csv",
         bpo::value(&params.tracked_addr_csv_fmts)
             ->default_value(std::vector<std::string>(1, default_tracked_addr_csv_fmt),
                             default_tracked_addr_csv_fmt)
             ->multitoken()
             ->zero_tokens(),
         "path(s) to output .csv file that contains tracked MAC addresses."
         " will be formatted by std::put_time().") //
        ("tracked-addr-html-in",
         bpo::value(&params.tracked_addr_html_in)->default_value("tracked_addresses.html.in"),
         "path to input .html file that will be used as a template") //
        ("tracked-addr-html",
         bpo::value(&params.tracked_addr_html_fmts)
             ->default_value(std::vector<std::string>(1, default_tracked_addr_html_fmt),
                             default_tracked_addr_html_fmt)
             ->multitoken()
             ->zero_tokens(),
         "path(s) to output .html file. will be formatted by std::put_time().") //
        ("arp-scan-options",
         bpo::value(&params.arp_scan_options)->default_value(mtt::Set::defaultOptions()),
         "options for arp-scan") //
        ("scan-interval",
         bpo::value<unsigned int>()->default_value(5)->notifier([&params](const unsigned int val) {
           params.scan_interval = std::chrono::minutes(val);
         }),
         "interval between MAC address scans in minutes") //
        ("track-interval",
         bpo::value<unsigned int>()
             ->default_value(60 * 24, "60 * 24")
             ->notifier([&params](const unsigned int val) {
               params.track_interval = std::chrono::minutes(val);
             }),
         "interval to rotate output .csv and .html files in minutes")              //
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

///////////////////
// Console outputs

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

void printTrackedAddresses(std::ostream &os, const mtt::PeriodMap &tracked_addrs,
                           const mtt::PeriodMap::Period &period) {
  const std::pair<mtt::PeriodMap::const_iterator, mtt::PeriodMap::const_iterator> range =
      tracked_addrs.equal_range(period);
  if (range.first != range.second) {
    os << "Tracked addresses" << std::endl;
    for (mtt::PeriodMap::const_iterator it = range.first; it != range.second; ++it) {
      os << "    " << it->second.address << " ('" << it->second.category << "' > '"
         << it->second.description << "')" << std::endl;
    }
  } else {
    os << "No tracked addresses" << std::endl;
  }
}

////////
// Misc

std::string readFile(const std::string &filename) {
  std::ifstream ifs(filename);
  if (!ifs) {
    throw std::runtime_error("Cannot open '" + filename + "' to read");
  }
  return std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
}

std::vector<std::string> format(const mtt::Time &formatter, const std::vector<std::string> &exprs) {
  std::vector<std::string> formatted;
  for (const std::string &expr : exprs) {
    formatted.push_back(formatter.toStr(expr));
  }
  return formatted;
}

// returns a present period p that meets (p.first <= now < p.second)
// where (p.first = base + n * interval) and (p.second = p.first + interval).
// base is 0:00 am, today in local time.
// i.e. returns {5:00, 6:00} if now is 5:10 and interval is 60 minutes,
//           or {10:20, 10:30} if now is 10:21 and interval is 10 minutes.
mtt::PeriodMap::Period findPresentPeriod(const std::chrono::minutes &interval) {
  // base
  const std::time_t now_ut = std::time(NULL); // now in universal time
  std::tm *base_lt = std::localtime(&now_ut); // base in local time
  base_lt->tm_hour = base_lt->tm_min = base_lt->tm_sec = 0;
  const mtt::Time base(std::chrono::seconds(std::mktime(base_lt))); // base in universal time

  // present period
  const int n = (mtt::Time::now() - base) / interval;
  const mtt::Time start = base + n * interval;
  return {start, start + interval};
}

////////
// Main

int main(int argc, char *argv[]) {
  // Parse command line args
  std::string help_msg;
  const Parameters params = Parameters::fromCommandLine(argc, argv, &help_msg);
  if (!help_msg.empty()) {
    std::cout << help_msg << std::endl;
    return 0;
  }

  // Tracking loop (never returns)
  for (int i_track = 0;; ++i_track) {
    // Constants and storage for this tracking period
    const mtt::PeriodMap::Period track_period = findPresentPeriod(params.track_interval);
    const std::vector<std::string> tracked_addr_csvs =
        format(track_period.first, params.tracked_addr_csv_fmts); // output .csv filenames
    const std::vector<std::string> tracked_addr_htmls =
        format(track_period.first, params.tracked_addr_html_fmts); // output .html filenames
    mtt::PeriodMap tracked_addrs;                                  // storage
    if (params.verbose) {
      std::cout << "Tracking period #" << i_track << "\n"
                << "     start: " << track_period.first << "\n"
                << "       end: " << track_period.second << "\n"
                << "    output: (csv) " << boost::algorithm::join(tracked_addr_csvs, ", ") << "\n"
                << "            (html) " << boost::algorithm::join(tracked_addr_htmls, ", ")
                << std::endl;
    }

    // Step 1: Load known addresses and a template of output .html from files
    mtt::AddressMap known_addrs;
    std::string tracked_addr_html_in;
    try {
      known_addrs = mtt::AddressMap::fromFile(params.known_addr_csv);
      if (params.verbose) {
        printKnownAddresses(std::cout, params.known_addr_csv, known_addrs);
      }
      if (!tracked_addr_htmls.empty()) {
        tracked_addr_html_in = readFile(params.tracked_addr_html_in);
      }
    } catch (const std::exception &err) {
      std::cerr << err.what() << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }

    // Scanning loop that will repeat until the end of this tracking period
    for (int i_scan = 0; mtt::Time::now() < track_period.second; ++i_scan) {
      // Constants for this scanning period
      const mtt::PeriodMap::Period scan_period = findPresentPeriod(params.scan_interval);
      if (params.verbose) {
        std::cout << "Scanning period #" << i_track << "." << i_scan << "\n"
                  << "    start: " << scan_period.first << "\n"
                  << "      end: " << scan_period.second << std::endl;
      }

      try {
        // Step 2: Scan addresses in network and match them to the known addresses
        const mtt::Set present_addrs = mtt::Set::fromARPScan(params.arp_scan_options);
        for (const mtt::Address &addr : present_addrs) {
          const mtt::AddressMap::const_iterator it = known_addrs.find(addr);
          if (it != known_addrs.end()) {
            tracked_addrs.insert(
                {scan_period, {addr, it->second.category, it->second.description}});
          }
        }
        if (params.verbose) {
          printTrackedAddresses(std::cout, tracked_addrs, scan_period);
        }

        // Step 3: Save scan results
        for (const std::string &csv : tracked_addr_csvs) {
          tracked_addrs.toFile(csv);
        }
        for (const std::string &html : tracked_addr_htmls) {
          tracked_addrs.toHTML(html, tracked_addr_html_in);
        }
      } catch (const std::exception &err) {
        std::cerr << err.what() << std::endl;
      }

      // Step 4: Sleep until the next scanning period
      std::this_thread::sleep_until(scan_period.second);
    }
  }

  return 0;
}
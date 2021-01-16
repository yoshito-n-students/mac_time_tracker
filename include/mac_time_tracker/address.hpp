#ifndef MAC_TIME_TRACKER_ADDRESS_HPP
#define MAC_TIME_TRACKER_ADDRESS_HPP

#include <array>
#include <cstdint>
#include <cstdio>
#include <regex>
#include <stdexcept>
#include <string>

#include <boost/algorithm/string/trim.hpp>

namespace mac_time_tracker {

///////////////
// MAC address

class Address : public std::array<std::uint8_t, 6> {
public:
  Address() {}
  Address(const std::uint8_t v0, const std::uint8_t v1, const std::uint8_t v2,
          const std::uint8_t v3, const std::uint8_t v4, const std::uint8_t v5) {
    (*this)[0] = v0;
    (*this)[1] = v1;
    (*this)[2] = v2;
    (*this)[3] = v3;
    (*this)[4] = v4;
    (*this)[5] = v5;
  }

  // make an instance from "00:AA:11:bb:22:Cc" or "0a-1B-2c-3D-4e-5F"
  static Address fromStr(const std::string &_str) {
    // remove leading and trailing spaces and check the length
    const std::string str = boost::trim_copy(_str);
    if (str.length() != 17) {
      throw std::runtime_error("Address::fromStr(): length of '" + str + "' is not 17");
    }

    Address addr;
    // sscanf returns number of items scanned, which is 6 on success
    //   "%2hhx"   -> 2 chars (2) of hex (x) to uint8_t (hh)
    //   "%*1[-:]" -> 1 char (1) of '-' or ':' to be ignored (*)
    if (std::sscanf(str.c_str(),
                    "%2hhx%*1[-:]%2hhx%*1[-:]%2hhx%*1[-:]%2hhx%*1[-:]%2hhx%*1[-:]%2hhx", &addr[0],
                    &addr[1], &addr[2], &addr[3], &addr[4], &addr[5]) != 6) {
      throw std::runtime_error("Address::fromStr(): Cannot parse '" + str + "'");
    }
    return addr;
  }

  // convert to string using the given separator char
  std::string toStr(const char sep = ':') const {
    char str[18];
    // "%02X" -> 2 chars (2) of hex in uppercase (X), 0-padded (0)
    std::sprintf(str, "%02X %02X %02X %02X %02X %02X", (*this)[0], (*this)[1], (*this)[2],
                 (*this)[3], (*this)[4], (*this)[5]);
    str[2] = str[5] = str[8] = str[11] = str[14] = sep;
    return str;
  }
};
} // namespace mac_time_tracker

#endif
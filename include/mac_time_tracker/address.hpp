#ifndef MAC_TIME_TRACKER_ADDRESS_HPP
#define MAC_TIME_TRACKER_ADDRESS_HPP

#include <array>
#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <string>

namespace mac_time_tracker {

///////////////
// MAC address

class Address : public std::array<std::uint8_t, 6> {
public:
  // make an instance from "00:AA:11:bb:22:Cc" or "0a-1B-2c-3D-4e-5F"
  static Address fromStr(const std::string &str) {
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
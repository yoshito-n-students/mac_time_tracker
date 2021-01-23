#ifndef MAC_TIME_TRACKER_ADDRESS_HPP
#define MAC_TIME_TRACKER_ADDRESS_HPP

#include <array>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>

#include <mac_time_tracker/io.hpp>

namespace mac_time_tracker {

///////////////
// MAC address

class Address : public std::array<std::uint8_t, 6>, public Readable<Address>, public Writable {
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

  using Writable::toStr;
  std::string toStr(const char sep) const {
    char str[18];
    // "%02X" -> 2 chars (2) of hex in uppercase (X), 0-padded (0)
    std::sprintf(str, "%02X %02X %02X %02X %02X %02X", (*this)[0], (*this)[1], (*this)[2],
                 (*this)[3], (*this)[4], (*this)[5]);
    str[2] = str[5] = str[8] = str[11] = str[14] = sep;
    return str;
  }

private:
  // read a string like "00:AA:11:bb:22:Cc" or "0a-1B-2c-3D-4e-5F" from the given stream
  virtual void read(std::istream &is) override {
    std::string str;
    is >> str;
    // sscanf returns number of items scanned, which is 6 on success
    //   "%2hhx"   -> 2 chars (2) of hex (x) to uint8_t (hh)
    //   "%*1[-:]" -> 1 char (1) of '-' or ':' to be ignored (*)
    if (str.size() != 17 ||
        std::sscanf(
            str.c_str(), "%2hhx%*1[-:]%2hhx%*1[-:]%2hhx%*1[-:]%2hhx%*1[-:]%2hhx%*1[-:]%2hhx",
            &(*this)[0], &(*this)[1], &(*this)[2], &(*this)[3], &(*this)[4], &(*this)[5]) != 6) {
      is.setstate(std::istream::failbit);
    }
  }

  // write a string like "00:AA:11:BB:22:CC" to the given stream
  virtual void write(std::ostream &os) const override { os << toStr(':'); }
};
} // namespace mac_time_tracker

#endif
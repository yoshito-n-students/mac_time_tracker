#ifndef MAC_TIME_TRACKER_IO_HPP
#define MAC_TIME_TRACKER_IO_HPP

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace mac_time_tracker {

template <class T> class Readable {
public:
  friend std::istream &operator>>(std::istream &is, Readable<T> &val) {
    val.read(is);
    return is;
  }

  static T fromStr(const std::string &str) {
    std::istringstream iss(str);
    T val;
    iss >> val;
    if (!iss) {
      throw std::runtime_error("Readable::fromStr(): Cannot parse '" + str + "'");
    }
    return val;
  }

  static T fromFile(const std::string &filename) {
    std::ifstream ifs(filename);
    if (!ifs) {
      throw std::runtime_error("Readable::fromFile(): Cannot open '" + filename + "'");
    }
    T val;
    ifs >> val;
    if (!ifs) {
      throw std::runtime_error("Readable::fromFile(): Cannot parse '" + filename + "'");
    }
    return val;
  }

private:
  virtual void read(std::istream &is) = 0;
};

class Writable {
public:
  friend std::ostream &operator<<(std::ostream &os, const Writable &val) {
    val.write(os);
    return os;
  }

  std::string toStr() const {
    std::ostringstream oss;
    write(oss);
    if (!oss) {
      throw std::runtime_error("Writable::toStr(): Cannot write to a string");
    }
    return oss.str();
  }

  void toFile(const std::string &filename) const {
    std::ofstream ofs(filename);
    if (!ofs) {
      throw std::runtime_error("Writable::toFile(): Cannot open '" + filename + "'");
    }
    write(ofs);
    if (!ofs) {
      throw std::runtime_error("Writable::toFile(): Cannot write to '" + filename + "'");
    }
  }

private:
  virtual void write(std::ostream &os) const = 0;
};
} // namespace mac_time_tracker

#endif
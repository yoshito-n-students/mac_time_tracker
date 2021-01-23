#ifndef MAC_TIME_TRACKER_TIME_HPP
#define MAC_TIME_TRACKER_TIME_HPP

#include <chrono>
#include <ctime>   // for std::localtime()
#include <iomanip> // for std::put_time()
#include <iostream>
#include <string>

#include <boost/lexical_cast.hpp>

#include <mac_time_tracker/io.hpp>

namespace mac_time_tracker {

class Time : public std::chrono::system_clock::time_point, public Writable {
private:
  using Base = std::chrono::system_clock::time_point;

public:
  // Constructors
  using Base::Base;
  Time(const Base &base) : Base(base) {}
  Time(Base &&base) : Base(base) {}
  template <class Duration>
  Time(const std::chrono::time_point<clock, Duration> &t)
      : Base(std::chrono::time_point_cast<Base>(t)) {}

  // A shortcut to Time::clock::now()
  static Time now() { return clock::now(); }

  using Writable::toStr;
  std::string toStr(const std::string &fmt) const {
    const std::time_t t = clock::to_time_t(*this);
    return boost::lexical_cast<std::string>(std::put_time(std::localtime(&t), fmt.c_str()));
  }

private:
  virtual void write(std::ostream &os) const override {
    // use a format like "Y-M-D H:M:S", which is based on ISO 8601
    os << toStr("%F %T");
  }
};

} // namespace mac_time_tracker

#endif
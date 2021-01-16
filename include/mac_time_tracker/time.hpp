#ifndef MAC_TIME_TRACKER_TIME_HPP
#define MAC_TIME_TRACKER_TIME_HPP

#include <chrono>
#include <ctime>   // for std::localtime()
#include <iomanip> // for std::put_time()
#include <string>

#include <boost/lexical_cast.hpp>

namespace mac_time_tracker {

class Time : public std::chrono::system_clock::time_point {
private:
  using Base = std::chrono::system_clock::time_point;

public:
  // Constructors
  using Base::Base;
  Time(const Base &base) : Base(base) {}
  Time(Base &&base) : Base(base) {}

  // Convert to string. The default format is based on ISO 8601 ("Y-M-D H:M:S")
  std::string toStr(const std::string &fmt = "%F %T") const {
    const std::time_t t = clock::to_time_t(*this);
    return boost::lexical_cast<std::string>(std::put_time(std::localtime(&t), fmt.c_str()));
  }
};

} // namespace mac_time_tracker

#endif
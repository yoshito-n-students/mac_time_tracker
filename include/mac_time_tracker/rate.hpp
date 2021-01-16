#ifndef MAC_TIME_TRACKER_RATE_HPP
#define MAC_TIME_TRACKER_RATE_HPP

#include <chrono>
#include <thread>

#include <mac_time_tracker/time.hpp>

namespace mac_time_tracker {

class Rate {
public:
  template <typename Duration>
  Rate(const Duration &period)
      : period_(std::chrono::duration_cast<Time::duration>(period)),
        start_time_(Time::clock::now()) {}

  Time::duration period() const { return period_; }

  Time startTime() const { return start_time_; }

  void sleep() {
    // A faster version of
    //   # const Time now = Time::clock::now();
    //   # while(start_time_ < now){
    //   #   start_time_ += period_;
    //   # }
    start_time_ += ((Time::clock::now() - start_time_) / period_ + 1) * period_;
    std::this_thread::sleep_until(start_time_);
  }

private:
  const Time::duration period_;
  Time start_time_;
};

} // namespace mac_time_tracker

#endif
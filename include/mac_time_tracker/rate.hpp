#ifndef MAC_TIME_TRACKER_RATE_HPP
#define MAC_TIME_TRACKER_RATE_HPP

#include <chrono>
#include <thread>

namespace mac_time_tracker {

class Rate {
private:
  // use a steady clock because the system clock may be syncronized or adjusted during operation
  using Clock = std::chrono::steady_clock;

public:
  using Time = Clock::time_point;
  using Duration = Clock::duration;

public:
  template <class Rep, class Ratio>
  Rate(const std::chrono::duration<Rep, Ratio> &period)
      : period_(std::chrono::duration_cast<Duration>(period)), start_time_(Clock::now()) {}

  Duration period() const { return period_; }

  Time startTime() const { return start_time_; }

  void sleep() {
    // A faster version of
    //   # const Time now = Clock::now();
    //   # while(start_time_ < now){
    //   #   start_time_ += period_;
    //   # }
    start_time_ += ((Clock::now() - start_time_) / period_ + 1) * period_;
    std::this_thread::sleep_until(start_time_);
  }

private:
  const Duration period_;
  Time start_time_;
};

} // namespace mac_time_tracker

#endif
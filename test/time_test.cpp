#include <chrono>
#include <regex>

#include <gtest/gtest.h>

#include <mac_time_tracker/time.hpp>

namespace mtt = mac_time_tracker;

TEST(Time, constructor) {
  // Default constructor
  ASSERT_EQ(mtt::Time(), mtt::Time(std::chrono::seconds(0)));
  // Construction with duration
  ASSERT_EQ(mtt::Time(std::chrono::minutes(60)), mtt::Time(std::chrono::hours(1)));
  ASSERT_NE(mtt::Time(std::chrono::seconds(1)), mtt::Time(std::chrono::milliseconds(1001)));
  // Duration construction param is time since epoch
  ASSERT_EQ(mtt::Time(std::chrono::hours(42)).time_since_epoch(), std::chrono::hours(42));
}

TEST(Time, toStr) {
  const mtt::Time t0, t1(std::chrono::hours(24));
  // Default format
  const std::regex re_default(R"(^\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}$)");
  ASSERT_TRUE(std::regex_match(t0.toStr(), re_default));
  ASSERT_TRUE(std::regex_match(t1.toStr(), re_default));
  ASSERT_STRNE(t0.toStr().c_str(), t1.toStr().c_str());
  // Custom format
  const std::string fmt_custom = "Day %u of week %V";
  const std::regex re_custom(R"(^Day [1-7] of week [0-5][0-9]$)");
  ASSERT_TRUE(std::regex_match(t0.toStr(fmt_custom), re_custom));
  ASSERT_TRUE(std::regex_match(t1.toStr(fmt_custom), re_custom));
  ASSERT_STRNE(t0.toStr(fmt_custom).c_str(), t1.toStr(fmt_custom).c_str());
}
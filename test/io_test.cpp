#include <iostream>
#include <sstream>
#include <string>
#include <utility> // for std::pair<>

#include <gtest/gtest.h>

#include <mac_time_tracker/io.hpp>

#include "make_temp_file.hpp"

namespace mtt = mac_time_tracker;

template <class T0, class T1>
class Pair : public std::pair<T0, T1>, public mtt::Readable<Pair<T0, T1>>, public mtt::Writable {
private:
  using Base = std::pair<T0, T1>;

public:
  using Base::Base;

private:
  virtual void read(std::istream &is) override { is >> Base::first >> Base::second; }

  virtual void write(std::ostream &os) const override {
    os << "{" << Base::first << ", " << Base::second << "}";
  }
};

TEST(IO, readable) {
  {
    std::istringstream iss("0 1");
    Pair<int, int> p;
    iss >> p;
    ASSERT_TRUE(iss);
    ASSERT_EQ(0, p.first);
    ASSERT_EQ(1, p.second);
  }
  {
    std::istringstream iss("0"); // no second token
    Pair<int, int> p;
    iss >> p;
    ASSERT_FALSE(iss);
    ASSERT_EQ(0, p.first);
  }
  {
    std::istringstream iss("12 34 foo bar hoge"); // too many tokens for p (but it's ok)
    Pair<Pair<int, int>, Pair<std::string, std::string>> p;
    iss >> p;
    ASSERT_TRUE(iss);
    ASSERT_EQ(12, p.first.first);
    ASSERT_EQ(34, p.first.second);
    ASSERT_STREQ("foo", p.second.first.c_str());
    ASSERT_STREQ("bar", p.second.second.c_str());
    std::string s;
    iss >> s;
    ASSERT_TRUE(iss);
    ASSERT_STREQ("hoge", s.c_str());
  }
  {
    using P = Pair<Pair<Pair<int, int>, std::string>, std::string>;
    const P p = P::fromFile(makeTempFile("12 34 foo bar"));
    ASSERT_EQ(12, p.first.first.first);
    ASSERT_EQ(34, p.first.first.second);
    ASSERT_STREQ("foo", p.first.second.c_str());
    ASSERT_STREQ("bar", p.second.c_str());
  }
}

TEST(IO, writable) {
  {
    const Pair<int, int> p = {0, 1};
    ASSERT_STREQ("{0, 1}", p.toStr().c_str());
  }
  {
    const Pair<Pair<int, int>, std::string> p = {{12, 34}, "foo"};
    ASSERT_STREQ("{{12, 34}, foo}", p.toStr().c_str());
  }
}
#include <iostream>
#include <sstream>
#include <stdexcept>
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

TEST(Readable, fromStream) {
  {
    std::istringstream iss("0 1");
    Pair<int, int> p;
    ASSERT_NO_THROW(iss >> p);
    ASSERT_TRUE(iss);
    ASSERT_EQ(0, p.first);
    ASSERT_EQ(1, p.second);
  }
  { // no second token
    std::istringstream iss("0");
    Pair<int, int> p;
    iss >> p;
    ASSERT_FALSE(iss);
    ASSERT_EQ(0, p.first);
  }
  { // random number of spaces between tokens and too many tokens for p, but it's ok
    std::istringstream iss("  12 \t34 \n foo bar   hoge  ");
    Pair<Pair<int, int>, Pair<std::string, std::string>> p;
    ASSERT_NO_THROW(iss >> p);
    ASSERT_TRUE(iss);
    ASSERT_EQ(12, p.first.first);
    ASSERT_EQ(34, p.first.second);
    ASSERT_STREQ("foo", p.second.first.c_str());
    ASSERT_STREQ("bar", p.second.second.c_str());
    std::string s;
    ASSERT_NO_THROW(iss >> s);
    ASSERT_TRUE(iss);
    ASSERT_STREQ("hoge", s.c_str());
  }
}

TEST(Readable, fromStr) {
  using P = Pair<int, int>;
  P p;
  ASSERT_THROW(p = P::fromStr("0"), std::runtime_error);
  ASSERT_THROW(p = P::fromStr("0 one"), std::runtime_error);
  ASSERT_NO_THROW(p = P::fromStr(" 0 1"));
  ASSERT_NO_THROW(p = P::fromStr("2 3 4"));
  ASSERT_EQ(2, p.first);
  ASSERT_EQ(3, p.second);
}

TEST(Readable, fromFile) {
  using P = Pair<Pair<Pair<int, int>, std::string>, std::string>;
  P p;
  ASSERT_THROW(p = P::fromFile("/path/that/does/not/exist"), std::runtime_error);
  ASSERT_THROW(p = P::fromFile(makeTempFile("foo bar 12 34")), std::runtime_error);
  ASSERT_NO_THROW(p = P::fromFile(makeTempFile(" 12 34 foo   bar")));
  ASSERT_EQ(12, p.first.first.first);
  ASSERT_EQ(34, p.first.first.second);
  ASSERT_STREQ("foo", p.first.second.c_str());
  ASSERT_STREQ("bar", p.second.c_str());
}

TEST(Writable, toStreamAndStr) {
  {
    const Pair<int, int> p = {0, 1};
    std::ostringstream oss;
    ASSERT_NO_THROW(oss << p);
    ASSERT_STREQ("{0, 1}", oss.str().c_str());
    ASSERT_STREQ("{0, 1}", p.toStr().c_str());
  }
  {
    const Pair<Pair<int, int>, std::string> p = {{12, 34}, "foo"};
    std::ostringstream oss;
    ASSERT_NO_THROW(oss << p);
    ASSERT_STREQ("{{12, 34}, foo}", oss.str().c_str());
    ASSERT_STREQ("{{12, 34}, foo}", p.toStr().c_str());
  }
}
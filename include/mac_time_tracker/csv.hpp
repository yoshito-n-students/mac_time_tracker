#ifndef MAC_TIME_TRACKER_CSV_HPP
#define MAC_TIME_TRACKER_CSV_HPP

#include <iostream>
#include <string>
#include <vector>

#include <boost/algorithm/string/replace.hpp>
#include <boost/tokenizer.hpp>

#include <mac_time_tracker/io.hpp>

namespace mac_time_tracker {

class CSV : public std::vector<std::vector<std::string>>, public Readable<CSV>, public Writable {
private:
  using Base = std::vector<std::vector<std::string>>;

public:
  using Base::Base;
  CSV(const Base &base) : Base(base) {}
  CSV(Base &&base) : Base(base) {}

private:
  // read CSV from the given string.
  // this implements a variant of CSV that
  //   - ends with an empty line or EOF
  //   - allows different number of fields between lines
  virtual void read(std::istream &is) override {
    clear();
    while (true) {
      // read a line from the stream
      std::string line;
      std::getline(is, line);
      if (line.empty()) {
        break;
      }
      // tokenize the line
      boost::tokenizer<boost::escaped_list_separator<char>> parser(line);
      emplace_back();
      for (const std::string &token : parser) {
        back().push_back(token);
      }
    }
    // this means successfully reached EOF but std::getline() set the fail flag
    // because the last line was empty. cancel the fail flag (i.e. only the eof flag)
    // because that is ok as a CSV format.
    if (is.fail() && !is.bad() && is.eof()) {
      is.clear(std::istream::eofbit);
    }
  }

  // dump data to the given stream
  virtual void write(std::ostream &os) const override {
    for (const std::vector<std::string> &line : *this) {
      for (std::size_t i = 0; i < line.size(); ++i) {
        if (i > 0) {
          os << ",";
        }
        std::string escaped = line[i];
        boost::replace_all(escaped, R"(\)", R"(\\)"); // escape ch
        boost::replace_all(escaped, R"(")", R"(\")"); // quote
        boost::replace_all(escaped, "\n", R"(\n)");   // new line
        os << "\"" << escaped << "\"";
      }
      os << "\n";
    }
  }
};

} // namespace mac_time_tracker

#endif
#ifndef MAC_TIME_TRACKER_CSV_HPP
#define MAC_TIME_TRACKER_CSV_HPP

#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/algorithm/string/replace.hpp>
#include <boost/tokenizer.hpp>

namespace mac_time_tracker {

class CSV : public std::vector<std::vector<std::string>> {
private:
  using Base = std::vector<std::vector<std::string>>;

public:
  using Base::Base;
  CSV(const Base &base) : Base(base) {}
  CSV(Base &&base) : Base(base) {}

  // create an instance from a file
  static CSV fromFile(const std::string &filename) {
    std::ifstream ifs(filename);
    if (!ifs) {
      throw std::runtime_error("CSV::fromFile(): Cannot open '" + filename + "'");
    }

    CSV csv;
    while (true) {
      // read a line from the file
      std::string line;
      if (!std::getline(ifs, line)) {
        break;
      }
      // tokenize the line
      boost::tokenizer<boost::escaped_list_separator<char>> parser(line);
      csv.emplace_back();
      for (const std::string &token : parser) {
        csv.back().push_back(token);
      }
    }
    return csv;
  }

  // dump data to a file
  void toFile(const std::string &filename) const {
    std::ofstream ofs(filename);
    if (!ofs) {
      throw std::runtime_error("CSV::toFile(): Cannot open '" + filename + "'");
    }

    for (const std::vector<std::string> &line : *this) {
      if (line.size() > 1) {
        ofs << "\"" << boost::replace_all_copy(line[0], "\"", "\\\"") // espcae "
            << "\"";
      }
      for (std::size_t i = 1; i < line.size(); ++i) {
        ofs << ",\"" << boost::replace_all_copy(line[i], "\"", "\\\"") << "\"";
      }
      ofs << "\n";
    }
  }
};

} // namespace mac_time_tracker

#endif
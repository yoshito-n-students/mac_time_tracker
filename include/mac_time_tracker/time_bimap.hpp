#ifndef MAC_TIME_TRACKER_TIME_BIMAP_HPP
#define MAC_TIME_TRACKER_TIME_BIMAP_HPP

#include <algorithm>
#include <set>
#include <string>
#include <utility> // for std::pair<>
#include <vector>

#include <boost/bimap/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>

#include <mac_time_tracker/address.hpp>
#include <mac_time_tracker/csv.hpp>
#include <mac_time_tracker/set.hpp>
#include <mac_time_tracker/time.hpp>

namespace mac_time_tracker {

/////////////////////////////////////////////////////////////
// Bimap of timestamp and category name with info of address
// to represent time history of address appearance

struct TimeBimapTraits {
  struct Tags {
    struct Time {};
    struct Category {};
    struct Address {};
  };

  template <class Type, class Tag> using Tagged = boost::bimaps::tagged<Type, Tag>;
  using TimeMultiset = boost::bimaps::multiset_of<Tagged<Time, Tags::Time>>;
  using CategoryMultiset = boost::bimaps::multiset_of<Tagged<std::string, Tags::Category>>;
  using WithAddress = boost::bimaps::with_info<Tagged<Address, Tags::Address>>;

  using Base = boost::bimaps::bimap<TimeMultiset, CategoryMultiset, WithAddress>;
};

class TimeBimap : public TimeBimapTraits::Base {
private:
  using Base = TimeBimapTraits::Base;

public:
  using Tags = TimeBimapTraits::Tags;

public:
  using Base::Base;
  TimeBimap(const Base &base) : Base(base) {}
  TimeBimap(Base &&base) : Base(base) {}

  // returns unique times in storage
  std::vector<Time> uniqueTimes() const {
    using TimeView = map_by<Tags::Time>::type;
    const TimeView &time_view = by<Tags::Time>();
    std::vector<Time> times;
    for (TimeView::const_iterator it = time_view.begin(); it != time_view.end();
         it = time_view.upper_bound(it->first)) {
      times.push_back(it->first);
    }
    return times;
  }

  // returns unique categories in storage
  std::vector<std::string> uniqueCategories() const {
    using CategoryView = map_by<Tags::Category>::type;
    const CategoryView &category_view = by<Tags::Category>();
    std::vector<std::string> categories;
    for (CategoryView::const_iterator it = category_view.begin(); it != category_view.end();
         it = category_view.upper_bound(it->first)) {
      categories.push_back(it->first);
    }
    return categories;
  }

  // returns addresses associated with the given time and category
  Set addressesAt(const Time &time, const std::string &category) const {
    // find range matching the given time and category
    const std::pair<const_iterator, const_iterator> range =
        std::equal_range(begin(), end(), value_type{time, category, Address()},
                         [this](const value_type &a, const value_type &b) -> bool {
                           return value_comp()(a, {b.left, b.right, a.info});
                         });
    // collect addresses in the range
    Set addr;
    for (const_iterator it = range.first; it != range.second; ++it) {
      addr.insert(it->info);
    }
    return addr;
  }

  // make a CSV like
  //       0: "Category", <t[0]>,    ... , <t[n]>
  //       1: <c[0]>,     <a[0][0]>, ... , <a[n][0]>
  //     ...
  //   m + 1: <c[m]>,     <a[0][m]>, ... , <a[n][m]>
  // whare t[i]    -> i-th time,
  //       c[j]    -> j-th category,
  //       a[i][j] -> set of addresses associated with t[i] and c[j]
  CSV toCSV() const {
    // Collect times and catecories in strage
    const std::vector<Time> times = uniqueTimes();
    const std::vector<std::string> categories = uniqueCategories();
    // The CSV
    CSV csv;
    // The first line
    csv.emplace_back();
    csv.back().emplace_back("Category");
    for (const Time &time : times) {
      csv.back().emplace_back(time.toStr());
    }
    // Other lines
    for (const std::string &category : categories) {
      csv.emplace_back();
      csv.back().emplace_back(category);
      for (const Time &time : times) {
        csv.back().emplace_back();
        std::string &str = csv.back().back();
        for (const Address &addr : addressesAt(time, category)) {
          if (!str.empty()) {
            str += ",";
          }
          str += addr.toStr();
        }
      }
    }
    return csv;
  }
};

} // namespace mac_time_tracker

#endif
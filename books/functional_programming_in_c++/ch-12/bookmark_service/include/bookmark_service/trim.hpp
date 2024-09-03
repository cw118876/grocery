#ifndef BOOKMARK_SERVICE_TRIM_HPP_
#define BOOKMARK_SERVICE_TRIM_HPP_

#include <algorithm>
#include <cctype>
#include <locale>
#include <string>

namespace bms {

namespace detail {
inline bool not_space(char c) {
  return !std::isspace(c, std::locale{});
}

inline std::string trim_left(std::string&& s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), detail::not_space));
  return s;
}

inline std::string trim_right(std::string&& s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), detail::not_space).base(),
          s.rbegin().base());
  return s;
}
}  // namespace detail

inline std::string trim(const std::string& msg) {
  return detail::trim_left(detail::trim_right(std::string{msg}));
}

}  // namespace bms

#endif  //  BOOKMARK_SERVICE_TRIM_HPP_

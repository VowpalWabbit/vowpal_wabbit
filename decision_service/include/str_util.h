#pragma once
#include <string>
#include <sstream>

namespace reinforcement_learning { namespace utility {

  template <typename Last>
  std::string concat(std::ostringstream& os, const Last& last) {
    os << last;
    return os.str();
  }

  template <typename First, typename ... Rest>
  std::string concat(std::ostringstream& os, const First& first, const Rest& ... rest) {
    os << first;
    return concat(os, rest...);
  }

  template <typename First, typename ... Rest>
  std::string concat(const First& first, const Rest& ... rest) {
    std::ostringstream os;
    return concat(os, first, rest...);
  }

  class str_util {
  public:
    static std::string& to_lower(std::string& sval);
    static std::string& ltrim(std::string& sval);
    static std::string& rtrim(std::string& sval);
    static std::string& trim(std::string& sval);
  };
}}

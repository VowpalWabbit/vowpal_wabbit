#pragma once
#include <string>
namespace reinforcement_learning { namespace utility {
  class str_util {
  public:
    static std::string& to_lower(std::string& sval);
    static std::string& ltrim(std::string& sval);
    static std::string& rtrim(std::string& sval);
    static std::string& trim(std::string& sval);
  };
}}

#include "str_util.h"
#include <algorithm>
#include <cctype>

using namespace std;

// Adapted from source: https://stackoverflow.com/a/217605/7964431

namespace reinforcement_learning { namespace utility {

  string& str_util::to_lower(string& sval) {
    transform(sval.begin(), sval.end(), sval.begin(), ::tolower);
    return sval;
  }

  string& str_util::ltrim(std::string& sval) {
    sval.erase(
      sval.begin(), 
      std::find_if(sval.begin(), sval.end(), [](unsigned char ch) {
          return !std::isspace(ch);
      })
    );
    return sval;
  }

  string& str_util::rtrim(std::string& sval) {
    sval.erase(
      std::find_if(sval.rbegin(), sval.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
      }).base(), 
      sval.end()
    );
    return sval;
  }

  std::string& str_util::trim(std::string& sval) {
    return ltrim(rtrim(sval));
  }

}}

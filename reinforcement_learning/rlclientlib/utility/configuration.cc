#include "configuration.h"
#include "str_util.h"

#include <cstdlib>

namespace reinforcement_learning { namespace utility {

  configuration::configuration():_pmap(new map_type()) {}
  configuration::~configuration() { delete _pmap; }

  configuration::configuration(const configuration& other) {
    _pmap = new map_type(*( other._pmap ));
  }

  configuration& configuration::operator=(const configuration& rhs) {
    if ( this != &rhs ) {
      _pmap = new map_type(*(rhs._pmap));
    }
    return *this;
  }

  configuration& configuration::operator=(configuration&& temp) noexcept {
    if ( this != &temp ) {
      auto& map = *_pmap;
      temp._pmap->swap(map);
    }
    return *this;
  }

  configuration::configuration(configuration&& temp) noexcept {
    _pmap = temp._pmap;
    temp._pmap = nullptr;
  }

  void configuration::set(const char* name, const char* value) {
    auto& map = *_pmap;
    map[name] = value;
  }

  const char* configuration::get(const char* name, const char* defval) const {
    auto& map = *_pmap;
    const auto it = map.find(name);
    if (it != map.end())
      return it->second.c_str();
    return defval;
  }

  int configuration::get_int(const char* name, const int defval) const {
    auto& map = *_pmap;
    const auto it = map.find(name);
    if (it != map.end())
      return atoi(it->second.c_str());
    return defval;
  }

  bool configuration::get_bool(const char* name, const bool defval) const {
    auto& map = *_pmap;
    const auto it = map.find(name);
    if ( it != map.end() ) {
      auto sval = it->second;
      str_util::trim(str_util::to_lower(sval));
      if ( sval == "true" ) return true;
      else if ( sval == "false" ) return false;
      else return defval; // value string is neither true nor false.  return default 
    }
    return defval;
  }

  float configuration::get_float(const char* name, float defval) const {
    auto& map = *_pmap;
    const auto it = map.find(name);
    if ( it != map.end() )
      return strtof(it->second.c_str(), nullptr);
    return defval;
  }
}}

std::ostream& operator<<(std::ostream& os, const reinforcement_learning::utility::configuration& cc) {
  os << "{" << std::endl;
  for ( const auto& v : *( cc._pmap ) ) {
    os << "  (" << v.first << ", " << v.second << ")" << std::endl;
  }
  os << "}" << std::endl;
  return os;
}


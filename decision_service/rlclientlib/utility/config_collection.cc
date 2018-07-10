#include "config_collection.h"
#include "str_util.h"

namespace reinforcement_learning { namespace utility {

  config_collection::config_collection():_pmap(new map_type()) {}
  config_collection::~config_collection() { delete _pmap; }

  config_collection::config_collection(const config_collection& other) {
    _pmap = new map_type(*( other._pmap ));
  }

  config_collection& config_collection::operator=(const config_collection& rhs) {
    if ( this != &rhs ) {
      _pmap = new map_type(*(rhs._pmap));
    }
    return *this;
  }

  config_collection& config_collection::operator=(config_collection&& temp) noexcept {
    if ( this != &temp ) {
      auto& map = *_pmap;
      temp._pmap->swap(map);
    }
    return *this;
  }

  config_collection::config_collection(config_collection&& temp) noexcept {
    _pmap = temp._pmap;
    temp._pmap = nullptr;
  }

  void config_collection::set(const char* name, const char* value) {
    auto& map = *_pmap;
    map[name] = value;
  }

  const char* config_collection::get(const char* name, const char* defval) const {
    auto& map = *_pmap;
    const auto it = map.find(name);
    if (it != map.end())
      return it->second.c_str();
    return defval;
  }

  int config_collection::get_int(const char* name, const int defval) const {
    auto& map = *_pmap;
    const auto it = map.find(name);
    if (it != map.end())
      return atoi(it->second.c_str());
    return defval;
  }

  bool config_collection::get_bool(const char* name, const bool defval) const {
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

  float config_collection::get_float(const char* name, float defval) const {
    auto& map = *_pmap;
    const auto it = map.find(name);
    if ( it != map.end() )
      return atof(it->second.c_str());
    return defval;
  }

}}

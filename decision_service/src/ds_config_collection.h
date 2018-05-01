#pragma once
#include <string>
#include <unordered_map>

namespace decision_service { namespace utility {

  class config_collection
  {
  public:
    void set(const char* name, const char* value);

    const char* get(const char* name, const char* defval) const;
    int get_int(const char* name, int defval) const;

  private:
    std::unordered_map<std::string, std::string> _map;
  };

}}
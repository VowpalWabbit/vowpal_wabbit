#pragma once
#include <string>
#include <unordered_map>

namespace reinforcement_learning { namespace utility {

  class config_collection
  {
  public:
    config_collection();
    ~config_collection();
    config_collection(const config_collection&);
    config_collection& operator=(const config_collection&);
    config_collection& operator=(config_collection&&) noexcept;
    config_collection(config_collection&&) noexcept;

    void set(const char* name, const char* value);
    const char* get(const char* name, const char* defval) const;
    int get_int(const char* name, int defval) const;

  private:
    using map_type = std::unordered_map<std::string, std::string>;
    map_type* _pmap;
  };
}}
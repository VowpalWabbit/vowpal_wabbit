#include "global_data.h"
#include "vw.h"

#include <string>

namespace VW {
  primitive_feature_space::primitive_feature_space(::vw& _all) : all(_all) {}

  primitive_feature_space::primitive_feature_space(::vw& _all, const std::string& _name, size_t _len)
    : all(_all), name(_name[0]), hash(hash_space(all, _name)), fs(_len)
  {
  }

  primitive_feature_space::primitive_feature_space(
    ::vw& _all, const std::string& _name, std::initializer_list<feature> features)
  : primitive_feature_space(_all, _name, features.size())
  {
    size_t i = 0;
    for (const auto& f : features) {
      fs[i] = f;
    }
  }

  primitive_feature_space::primitive_feature_space(
    ::vw& _all, const std::string& _name, std::initializer_list<std::tuple<const char*, float>> features)
  : primitive_feature_space(_all, _name, features.size())
  {
    size_t i = 0;
    for (const auto& f : features)
    {
      set(i, std::get<0>(f), std::get<1>(f));
    }
  }

  void primitive_feature_space::reset(size_t _len) { fs.resize(_len); }

  const feature& primitive_feature_space::operator[](size_t index) const { return fs[index]; }
  feature& primitive_feature_space::operator[](size_t index) { return fs[index]; }

  void primitive_feature_space::set(size_t index, const std::string& feature_name, float value)
  {
    fs[index].weight_index = hash_feature(all, feature_name, hash);
    fs[index].x = value;
  }

  void primitive_feature_space::set_name(const std::string& _name)
  {
    name = _name[0];
    hash = hash_space(all, _name);
  }
  unsigned char primitive_feature_space::get_name() const { return name; }
  size_t primitive_feature_space::size() const { return fs.size(); }

  feature_space::feature_space(vw& _all, size_t _size) : all(_all), fspaces(_size, all) {}

  const primitive_feature_space& feature_space::operator[](size_t index) const { return fspaces[index]; }
  primitive_feature_space& feature_space::operator[](size_t index) { return fspaces[index]; }
}

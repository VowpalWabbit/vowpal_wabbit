#pragma once

template <bool audit>
struct Namespace
{
  char feature_group;
  feature_index namespace_hash;
  features* ftrs;
  size_t feature_count;
  const char* name;

  void AddFeature(feature_value v, feature_index i, const char* feature_name)
  {
    // filter out 0-values
    if (v == 0)
      return;

    ftrs->push_back(v, i);
    feature_count++;

    if (audit)
      ftrs->space_names.push_back(audit_strings_ptr(new audit_strings(name, feature_name)));
  }

  void AddFeature(vw* all, const char* str)
  {
    ftrs->push_back(1., VW::hash_feature_cstr(*all, const_cast<char*>(str), namespace_hash));
    feature_count++;

    if (audit)
      ftrs->space_names.push_back(audit_strings_ptr(new audit_strings(name, str)));
  }

  void AddFeature(vw* all, const char* key, const char* value)
  {
    ftrs->push_back(1., VW::hash_feature(*all, value, VW::hash_feature(*all, key, namespace_hash)));
    feature_count++;

    std::stringstream ss;
    ss << key << "^" << value;
    if (audit)
      ftrs->space_names.push_back(audit_strings_ptr(new audit_strings(name, ss.str())));
  }
};

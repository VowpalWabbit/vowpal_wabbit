#pragma once

// Decision Service JSON header information - required to construct final label
struct DecisionServiceInteraction
{
  std::string eventId;
  std::vector<unsigned> actions;
  std::vector<float> probabilities;
  float probabilityOfDrop = 0.f;
  bool skipLearn{false};
};

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
    if (v == 0) return;

    ftrs->push_back(v, i);
    feature_count++;

    if (audit) ftrs->space_names.push_back(audit_strings_ptr(new audit_strings(name, feature_name)));
  }

  void AddFeature(vw* all, const char* str)
  {
    ftrs->push_back(1., VW::hash_feature_cstr(*all, const_cast<char*>(str), namespace_hash));
    feature_count++;

    if (audit) ftrs->space_names.push_back(audit_strings_ptr(new audit_strings(name, str)));
  }

  void AddFeature(vw* all, const char* key, const char* value)
  {
    ftrs->push_back(1., VW::chain_hash(*all, key, value, namespace_hash));
    feature_count++;

    std::stringstream ss;
    ss << key << "^" << value;
    if (audit) ftrs->space_names.push_back(audit_strings_ptr(new audit_strings(name, ss.str())));
  }
};

template <bool audit>
void push_ns(example* ex, const char* ns, std::vector<Namespace<audit>>& namespaces, vw& all)
{
  Namespace<audit> n;
  n.feature_group = ns[0];
  n.namespace_hash = VW::hash_space_cstr(all, ns);
  n.ftrs = ex->feature_space.data() + ns[0];
  n.feature_count = 0;
  n.name = ns;
  namespaces.push_back(std::move(n));
}

template <bool audit>
void pop_ns(example* ex, std::vector<Namespace<audit>>& namespaces)
{
  auto& ns = namespaces.back();
  if (ns.feature_count > 0)
  {
    auto feature_group = ns.feature_group;
    // Do not insert feature_group if it already exists.
    if (std::find(ex->indices.begin(), ex->indices.end(), feature_group) == ex->indices.end())
    {
      ex->set_namespace(feature_group);
      // ex->indices.push_back(feature_group);
    }
  }
  namespaces.pop_back();
}

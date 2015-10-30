namespace LabelDict
{
struct feature_audit
{ v_array<feature> features;
  v_array<audit_data> audit;
};
typedef v_hashmap< size_t, feature_audit > label_feature_map;
inline bool size_t_eq(size_t &a, size_t &b) { return (a==b); }

void add_example_namespace(example& ec, char ns, v_array<feature>& features, v_array<audit_data>* audit);
void del_example_namespace(example& ec, char ns, v_array<feature>& features, bool audit);

void set_label_features(label_feature_map& lfm, size_t lab, v_array<feature>& features, v_array<audit_data>* audit);

void add_example_namespaces_from_example(example& target, example& source, bool audit);
void del_example_namespaces_from_example(example& target, example& source, bool audit);
void add_example_namespace_from_memory(label_feature_map& lfm, example& ec, size_t lab, bool audit);
void del_example_namespace_from_memory(label_feature_map& lfm, example& ec, size_t lab, bool audit);

void free_label_features(label_feature_map& lfm);
}

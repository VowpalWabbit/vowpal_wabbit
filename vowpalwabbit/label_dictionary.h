namespace LabelDict {
  typedef v_hashmap< size_t, v_array<feature> > label_feature_map;
  inline bool size_t_eq(size_t &a, size_t &b) { return (a==b); }

  void add_example_namespace(example& ec, char ns, v_array<feature> features);
  void del_example_namespace(example& ec, char ns, v_array<feature> features);

  void set_label_features(label_feature_map& data, size_t lab, v_array<feature>features);

  void add_example_namespaces_from_example(example& target, example& source);
  void del_example_namespaces_from_example(example& target, example& source);
  void add_example_namespace_from_memory(label_feature_map& data, example& ec, size_t lab);
  void del_example_namespace_from_memory(label_feature_map& lfm, example& ec, size_t lab);

  void free_label_features(label_feature_map& data);
}

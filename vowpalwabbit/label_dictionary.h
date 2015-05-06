namespace LabelDict {
  typedef v_hashmap< size_t, v_array<feature> > label_feature_map;
  inline bool size_t_eq(size_t &a, size_t &b) { return (a==b); }

  void add_example_namespace(example& ec, char ns, v_array<feature> features);
  void del_example_namespace(example& ec, char ns, v_array<feature> features);
  bool ec_is_label_definition(example& ec); // label defs look like "0:___" or just "label:___"
  bool ec_seq_is_label_definition(v_array<example*>ec_seq);

  bool ec_is_example_header(example& ec);  // example headers look like "0:-1" or just "shared"
  void set_label_features(label_feature_map& data, size_t lab, v_array<feature>features);

  void add_example_namespaces_from_example(example& target, example& source);
  void del_example_namespaces_from_example(example& target, example& source);
  void add_example_namespace_from_memory(label_feature_map& data, example& ec, size_t lab);
  void del_example_namespace_from_memory(label_feature_map& lfm, example& ec, size_t lab);

  void free_label_features(label_feature_map& data);
}

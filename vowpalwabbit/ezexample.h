// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include <cstdio>
#include "parser.h"
#include "vw.h"

typedef uint32_t fid;

struct vw_namespace
{
  char namespace_letter;

 public:
  vw_namespace(const char c) : namespace_letter(c) {}
};

class ezexample
{
 private:
  vw* vw_ref;
  vw* vw_par_ref;  // an extra parser if we're multithreaded
  bool is_multiline;

  char str[2];
  example* ec;
  bool we_create_ec;
  std::vector<fid> past_seeds;
  fid current_seed;
  size_t quadratic_features_num;
  float quadratic_features_sqr;
  char current_ns;
  bool ns_exists[256];
  bool example_changed_since_prediction;

  v_array<example*> example_copies;

  ezexample(const ezexample& ex) = delete;
  ezexample& operator=(const ezexample& ex) = delete;

  example* get_new_example()
  {
    example* new_ec = VW::new_unused_example(*vw_par_ref);
    vw_par_ref->p->lp.default_label(&new_ec->l);
    new_ec->tag.clear();
    new_ec->indices.clear();
    for (auto& i : new_ec->feature_space) i.clear();

    new_ec->ft_offset = 0;
    new_ec->num_features = 0;
    new_ec->partial_prediction = 0.;
    new_ec->updated_prediction = 0.;
    new_ec->passthrough = nullptr;
    new_ec->loss = 0.;
    new_ec->total_sum_feat_sq = 0.;
    new_ec->confidence = 0.;
    return new_ec;
  }

  void setup_new_ezexample(vw* this_vw, bool multiline, vw* this_vw_parser)
  {
    vw_ref = this_vw;
    vw_par_ref = (this_vw_parser == nullptr) ? this_vw : this_vw_parser;
    is_multiline = multiline;

    str[0] = 0;
    str[1] = 0;
    current_seed = 0;
    current_ns = 0;

    quadratic_features_num = 0;
    quadratic_features_sqr = 0.;

    for (bool& ns_exist : ns_exists) ns_exist = false;

    example_changed_since_prediction = true;
  }

  void setup_for_predict()
  {
    static example* empty_example = is_multiline ? VW::read_example(*vw_par_ref, (char*)"") : nullptr;
    if (example_changed_since_prediction)
    {
      mini_setup_example();
      vw_ref->learn(*ec);
      if (is_multiline)
        vw_ref->learn(*empty_example);
      example_changed_since_prediction = false;
    }
  }

 public:
  // REAL FUNCTIONALITY
  // create a new ezexample by asking the vw parser for an example
  ezexample(vw* this_vw, bool multiline = false, vw* this_vw_parser = nullptr)
  {
    setup_new_ezexample(this_vw, multiline, this_vw_parser);
    example_copies = v_init<example*>();
    ec = get_new_example();
    we_create_ec = true;

    if (vw_ref->add_constant)
      VW::add_constant_feature(*vw_ref, ec);
  }

  // create a new ezexample by wrapping around an already existing example
  // we do NOT copy your data, therefore, WARNING:
  //   do NOT touch the underlying example unless you really know what you're done)
  ezexample(vw* this_vw, example* this_ec, bool multiline = false, vw* this_vw_parser = nullptr)
  {
    setup_new_ezexample(this_vw, multiline, this_vw_parser);

    ec = this_ec;
    we_create_ec = false;

    for (auto ns : ec->indices) ns_exists[ns] = true;
    if (current_ns != 0)
    {
      str[0] = current_ns;
      current_seed = VW::hash_space(*vw_ref, str);
    }
  }

  ~ezexample()  // calls finish_example *only* if we created our own example!
  {
    if (ec->in_use && VW::is_ring_example(*vw_par_ref, ec))
      VW::finish_example(*vw_par_ref, *ec);
    for (auto ecc : example_copies)
      if (ecc->in_use && VW::is_ring_example(*vw_par_ref, ec))
        VW::finish_example(*vw_par_ref, *ecc);
    example_copies.clear();
    free(example_copies.begin());
  }

  bool ensure_ns_exists(char c)  // returns TRUE iff we should ignore it :)
  {
    if (vw_ref->ignore_some && vw_ref->ignore[(int)c])
      return true;
    if (ns_exists[(int)c])
      return false;
    ec->indices.push_back((size_t)c);
    ns_exists[(int)c] = true;
    return false;
  }

  void addns(char c)
  {
    if (ensure_ns_exists(c))
      return;

    ec->feature_space[(int)c].clear();
    past_seeds.push_back(current_seed);
    current_ns = c;
    str[0] = c;
    current_seed = VW::hash_space(*vw_ref, str);
  }

  void remns()
  {
    if (ec->indices.empty())
    {
      current_seed = 0;
      current_ns = 0;
    }
    else
    {
      if (ns_exists[(int)current_ns])
      {
        ec->total_sum_feat_sq -= ec->feature_space[(int)current_ns].sum_feat_sq;
        ec->feature_space[(int)current_ns].clear();
        ec->num_features -= ec->feature_space[(int)current_ns].size();

        ns_exists[(int)current_ns] = false;
      }

      current_seed = past_seeds.back();
      past_seeds.pop_back();
      ec->indices.pop();
      example_changed_since_prediction = true;
    }
  }

  inline fid addf(char to_ns, fid fint, float v)
  {
    if (to_ns == 0)
      return 0;
    if (ensure_ns_exists(to_ns))
      return 0;

    ec->feature_space[(int)to_ns].push_back(v, fint << vw_ref->weights.stride_shift());
    ec->total_sum_feat_sq += v * v;
    ec->num_features++;
    example_changed_since_prediction = true;
    return fint;
  }

  inline fid addf(fid fint, float v) { return addf(current_ns, fint, v); }

  // copy an entire namespace from this other example, you can even give it a new namespace name if you want!
  void add_other_example_ns(example& other, char other_ns, char to_ns)
  {
    if (ensure_ns_exists(to_ns))
      return;
    features& fs = other.feature_space[(int)other_ns];
    for (size_t i = 0; i < fs.size(); i++) ec->feature_space[(int)to_ns].push_back(fs.values[i], fs.indicies[i]);
    ec->total_sum_feat_sq += fs.sum_feat_sq;
    ec->num_features += fs.size();
    example_changed_since_prediction = true;
  }
  void add_other_example_ns(example& other, char ns)  // default to_ns to other_ns
  {
    add_other_example_ns(other, ns, ns);
  }

  void add_other_example_ns(ezexample& other, char other_ns, char to_ns)
  {
    add_other_example_ns(*other.ec, other_ns, to_ns);
  }
  void add_other_example_ns(ezexample& other, char ns) { add_other_example_ns(*other.ec, ns); }

  inline ezexample& set_label(std::string label)
  {
    VW::parse_example_label(*vw_par_ref, *ec, label);
    example_changed_since_prediction = true;
    return *this;
  }

  void mini_setup_example()
  {
    ec->partial_prediction = 0.;
    ec->weight = vw_par_ref->p->lp.get_weight(&ec->l);

    ec->num_features -= quadratic_features_num;
    ec->total_sum_feat_sq -= quadratic_features_sqr;

    quadratic_features_num = 0;
    quadratic_features_sqr = 0.;

    for (auto const& pair : vw_ref->pairs)
    {
      quadratic_features_num += ec->feature_space[(int)pair[0]].size() * ec->feature_space[(int)pair[1]].size();
      quadratic_features_sqr +=
          ec->feature_space[(int)pair[0]].sum_feat_sq * ec->feature_space[(int)pair[1]].sum_feat_sq;
    }
    ec->num_features += quadratic_features_num;
    ec->total_sum_feat_sq += quadratic_features_sqr;
  }

  size_t get_num_features() { return ec->num_features; }

  example* get()
  {
    if (example_changed_since_prediction)
      mini_setup_example();
    return ec;
  }

  float predict()
  {
    setup_for_predict();
    return ec->pred.scalar;
  }

  float predict_partial()
  {
    setup_for_predict();
    return ec->partial_prediction;
  }

  void train()  // if multiline, add to stack; otherwise, actually train
  {
    if (example_changed_since_prediction)
    {
      mini_setup_example();
      example_changed_since_prediction = false;
    }

    if (!is_multiline)
    {
      vw_ref->learn(*ec);
    }
    else  // is multiline
    {     // we need to make a copy
      example* copy = get_new_example();
      assert(ec->in_use);
      VW::copy_example_data(vw_ref->audit, copy, ec, vw_par_ref->p->lp.label_size, vw_par_ref->p->lp.copy_label);
      assert(copy->in_use);
      vw_ref->learn(*copy);
      example_copies.push_back(copy);
    }
  }

  void clear_features()
  {
    for (size_t i = 0; i < 256; i++)
    {
      if (current_ns == 0)
        break;
      remns();
    }
  }

  void finish()
  {
    static example* empty_example = is_multiline ? VW::read_example(*vw_par_ref, (char*)"") : nullptr;
    if (is_multiline)
    {
      vw_ref->learn(*empty_example);
      for (auto ecc : example_copies)
        if (ecc->in_use)
          VW::finish_example(*vw_par_ref, *ecc);
      example_copies.clear();
    }
  }

  // HELPER FUNCTIONALITY

  inline fid hash(std::string fstr) { return VW::hash_feature(*vw_ref, fstr, current_seed); }
  inline fid hash(char* fstr) { return VW::hash_feature_cstr(*vw_ref, fstr, current_seed); }
  inline fid hash(char c, std::string fstr)
  {
    str[0] = c;
    return VW::hash_feature(*vw_ref, fstr, VW::hash_space(*vw_ref, str));
  }
  inline fid hash(char c, char* fstr)
  {
    str[0] = c;
    return VW::hash_feature_cstr(*vw_ref, fstr, VW::hash_space(*vw_ref, str));
  }

  inline fid addf(fid fint) { return addf(fint, 1.0); }
  inline fid addf(std::string fstr, float val) { return addf(hash(fstr), val); }
  inline fid addf(std::string fstr) { return addf(hash(fstr), 1.0); }

  inline fid addf(char ns, fid fint) { return addf(ns, fint, 1.0); }
  inline fid addf(char ns, std::string fstr, float val) { return addf(ns, hash(ns, fstr), val); }
  inline fid addf(char ns, std::string fstr) { return addf(ns, hash(ns, fstr), 1.0); }

  inline ezexample& operator()(const vw_namespace& n)
  {
    addns(n.namespace_letter);
    return *this;
  }

  inline ezexample& operator()(fid fint)
  {
    addf(fint, 1.0);
    return *this;
  }
  inline ezexample& operator()(std::string fstr)
  {
    addf(fstr, 1.0);
    return *this;
  }
  inline ezexample& operator()(const char* fstr)
  {
    addf(fstr, 1.0);
    return *this;
  }
  inline ezexample& operator()(fid fint, float val)
  {
    addf(fint, val);
    return *this;
  }
  inline ezexample& operator()(std::string fstr, float val)
  {
    addf(fstr, val);
    return *this;
  }
  inline ezexample& operator()(const char* fstr, float val)
  {
    addf(fstr, val);
    return *this;
  }

  inline ezexample& operator()(char ns, fid fint)
  {
    addf(ns, fint, 1.0);
    return *this;
  }
  inline ezexample& operator()(char ns, std::string fstr)
  {
    addf(ns, fstr, 1.0);
    return *this;
  }
  inline ezexample& operator()(char ns, const char* fstr)
  {
    addf(ns, fstr, 1.0);
    return *this;
  }
  inline ezexample& operator()(char ns, fid fint, float val)
  {
    addf(ns, fint, val);
    return *this;
  }
  inline ezexample& operator()(char ns, std::string fstr, float val)
  {
    addf(ns, fstr, val);
    return *this;
  }
  inline ezexample& operator()(char ns, const char* fstr, float val)
  {
    addf(ns, fstr, val);
    return *this;
  }

  inline ezexample& operator()(example& other, char other_ns, char to_ns)
  {
    add_other_example_ns(other, other_ns, to_ns);
    return *this;
  }
  inline ezexample& operator()(example& other, char ns)
  {
    add_other_example_ns(other, ns);
    return *this;
  }
  inline ezexample& operator()(ezexample& other, char other_ns, char to_ns)
  {
    add_other_example_ns(other, other_ns, to_ns);
    return *this;
  }
  inline ezexample& operator()(ezexample& other, char ns)
  {
    add_other_example_ns(other, ns);
    return *this;
  }

  inline ezexample& operator--()
  {
    remns();
    return *this;
  }

  inline float operator()() { return predict(); }
};

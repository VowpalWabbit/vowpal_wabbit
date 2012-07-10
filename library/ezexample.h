#ifndef EZEXAMPLE_H
#define EZEXAMPLE_H

#include <stdio.h>
#include "../vowpalwabbit/vw.h"

using namespace std;
typedef uint32_t fid;

struct vw_namespace {
  char namespace_letter;
public: vw_namespace(const char c) : namespace_letter(c) {}
};

class ezexample {
private:
  vw*vw_ref;
  vector<VW::feature_space> *dat;
  vector<fid> past_seeds;
  fid current_seed;
  vector<feature>*current_ns;
  char str[2];
  bool pass_empty;
  string mylabel;
  ezexample(const ezexample & ex);
  ezexample & operator=(const ezexample & ex);

public:

  // REAL FUNCTIONALITY
  ezexample(vw*this_vw, bool pe=false) { 
    dat = new vector<VW::feature_space>();
    vw_ref = this_vw;
    current_seed = 0;
    current_ns = NULL;
    str[0] = ' '; str[1] = 0;
    pass_empty = pe;
    mylabel = "";
  }

  ~ezexample() {
    if (dat != NULL)
      delete dat;
  }

  void addns(char c) {
    str[0] = c;
    dat->push_back( VW::feature_space(c, vector<feature>()) );
    current_ns = &( dat->at(dat->size()-1).second );
    past_seeds.push_back(current_seed);
    current_seed = VW::hash_space(*vw_ref, str);
  }

  void remns() { 
    if (dat->size() == 0) {
      current_seed = 0;
      current_ns   = NULL;
    } else {
      current_seed = past_seeds.back();
      past_seeds.pop_back();
      dat->pop_back();
      current_ns = &(dat->back().second);
    }
  }

  inline fid addf(fid fint, float val) {
    if (!current_ns) return 0;
    feature f = { val, fint };
    current_ns->push_back(f);
    return fint;
  }

  inline ezexample& set_label(string label) { 
    mylabel = label; 
    return *this;
  }


  float predict() {
    static example* empty_example = VW::read_example(*vw_ref, (char*)"| "); 
    example *ec = VW::import_example(*vw_ref, *dat);

    if (mylabel.length() > 0)
      VW::parse_example_label(*vw_ref, *ec, mylabel);

    vw_ref->learn(vw_ref, ec);
    if (pass_empty) vw_ref->learn(vw_ref, empty_example);
    float pred = ec->final_prediction;
    VW::finish_example(*vw_ref, ec);
    return pred;
  }

  void print() {
    cerr << "ezexample dat->size=" << dat->size() << ", current_seed=" << current_seed << endl;
    for (size_t i=0; i<dat->size(); i++) {
      cerr << "  namespace(" << dat->at(i).first << "):" << endl;
      for (size_t j=0; j<dat->at(i).second.size(); j++) {
        cerr << "    " << dat->at(i).second[j].weight_index << "\t: " << dat->at(i).second[j].x << endl;
      }
    }
  }

  // HELPER FUNCTIONALITY
  inline fid hash(string fstr) { 
    return VW::hash_feature(*vw_ref, fstr, current_seed); 
  }
  inline fid hash(char* fstr) { 
    return VW::hash_feature_cstr(*vw_ref, fstr, current_seed);
  }
  inline fid hash(char c, string fstr) { 
    str[0] = c;
    return VW::hash_feature(*vw_ref, fstr, VW::hash_space(*vw_ref, str)); 
  }
  inline fid hash(char c, char* fstr) { 
    str[0] = c;
    return VW::hash_feature_cstr(*vw_ref, fstr, VW::hash_space(*vw_ref, str)); 
  }

  inline fid addf(fid    fint           ) { return addf(fint      , 1.0); }
  inline fid addf(string fstr, float val) { return addf(hash(fstr), val); }
  inline fid addf(string fstr           ) { return addf(hash(fstr), 1.0); }

  inline ezexample& operator()(fid         fint           ) { addf(fint, 1.0); return *this; }
  inline ezexample& operator()(string      fstr           ) { addf(fstr, 1.0); return *this; }
  inline ezexample& operator()(const char* fstr           ) { addf(fstr, 1.0); return *this; }
  inline ezexample& operator()(fid         fint, float val) { addf(fint, val); return *this; }
  inline ezexample& operator()(string      fstr, float val) { addf(fstr, val); return *this; }
  inline ezexample& operator()(const char* fstr, float val) { addf(fstr, val); return *this; }
  inline ezexample& operator()(const vw_namespace&n) { addns(n.namespace_letter); return *this; }
  inline ezexample& operator--() { remns(); return *this; }
  inline float      operator()() { return predict(); }
};

class ezexample_new {
 private:
  vw*vw_ref;
  bool pass_empty;

  char str[2];
  example*ec;
  vector<fid> past_seeds;
  fid current_seed;
  size_t quadratic_features_num;
  float quadratic_features_sqr;
  char current_ns;
  bool ignore_this_ns;

  bool example_changed_since_prediction;

  ezexample_new(const ezexample_new & ex);
  ezexample_new & operator=(const ezexample_new & ex);

 public:

  // REAL FUNCTIONALITY
  ezexample_new(vw*this_vw, bool pe=false) {
    vw_ref = this_vw;
    pass_empty = pe;

    str[0] = 0; str[1] = 0;
    current_seed = 0;
    current_ns = 0;
    ignore_this_ns = true;

    ec = VW::new_unused_example(*vw_ref);
    vw_ref->p->lp->default_label(ec->ld);
    ec->pass = vw_ref->passes_complete;

    quadratic_features_num = 0;
    quadratic_features_sqr = 0.;

    if (vw_ref->add_constant) {
      size_t cns = VW::get_constant_namespace();
      push(ec->indices, cns);
      feature temp = {1,(uint32_t) (VW::get_constant() & vw_ref->parse_mask)};
      push(ec->atomics[cns], temp);
      ec->total_sum_feat_sq++;
      ec->num_features++;
    }
    example_changed_since_prediction = true;
  }

  ~ezexample_new() {
    VW::finish_example(*vw_ref, ec);
  }

  void addns(char c) {
    past_seeds.push_back(current_seed);
    current_ns = c;
    str[0] = c;
    current_seed = VW::hash_space(*vw_ref, str);
    push(ec->indices, (size_t)c);
    ignore_this_ns = vw_ref->ignore_some && vw_ref->ignore[c];
  }

  void remns() {
    if (ec->indices.index() == 0) {
      current_seed = 0;
      current_ns = 0;
    } else {
      ec->total_sum_feat_sq -= ec->sum_feat_sq[current_ns];
      ec->sum_feat_sq[current_ns] = 0;
      ec->num_features -= ec->atomics[current_ns].index();
      ec->atomics[current_ns].erase();

      current_seed = past_seeds.back();
      past_seeds.pop_back();
      ec->indices.pop();
      example_changed_since_prediction = true;
    }
  }

  inline fid addf(char to_ns, fid fint, float v) {
    // TODO: make sure ns exists!
    if (!to_ns) return 0;
    if ((to_ns == current_ns) && ignore_this_ns) return 0;
    if (vw_ref->ignore_some && vw_ref->ignore[to_ns]) return 0;
    feature f = { v, fint * vw_ref->stride };
    push(ec->atomics[to_ns], f);
    ec->sum_feat_sq[to_ns] += v * v;
    ec->total_sum_feat_sq += v * v;
    ec->num_features++;
    example_changed_since_prediction = true;
  }

  inline fid addf(fid fint, float v) { return addf(current_ns, fint, v); }

  inline ezexample_new& set_label(string label) {
    VW::parse_example_label(*vw_ref, *ec, label);
    ec->global_weight = vw_ref->p->lp->get_weight(ec->ld);
    example_changed_since_prediction = true;
    return *this;
  }

  void mini_setup_example() {
    ec->partial_prediction = 0.;
    vw_ref->sd->t += ec->global_weight;
    ec->example_t = vw_ref->sd->t;

    ec->num_features      -= quadratic_features_num;
    ec->total_sum_feat_sq -= quadratic_features_sqr;

    quadratic_features_num = 0;
    quadratic_features_sqr = 0.;

    for (vector<string>::iterator i = vw_ref->pairs.begin(); i != vw_ref->pairs.end(); i++) {
      quadratic_features_num
        += (ec->atomics[(int)(*i)[0]].end - ec->atomics[(int)(*i)[0]].begin)
        *  (ec->atomics[(int)(*i)[1]].end - ec->atomics[(int)(*i)[1]].begin);
      quadratic_features_sqr
        += ec->sum_feat_sq[(int)(*i)[0]]
        *  ec->sum_feat_sq[(int)(*i)[1]];
    }
    ec->num_features      += quadratic_features_num;
    ec->total_sum_feat_sq += quadratic_features_sqr;
  }

  float predict() {
    static example* empty_example = pass_empty ? VW::read_example(*vw_ref, (char*)"| ") : NULL;
    if (example_changed_since_prediction) {
      mini_setup_example();
      vw_ref->learn(vw_ref, ec);
      if (pass_empty) vw_ref->learn(vw_ref, empty_example);
      example_changed_since_prediction = false;
    }
    return ec->final_prediction;
  }

  // HELPER FUNCTIONALITY
  inline fid hash(string fstr) { 
    return VW::hash_feature(*vw_ref, fstr, current_seed); 
  }
  inline fid hash(char* fstr) { 
    return VW::hash_feature_cstr(*vw_ref, fstr, current_seed);
  }
  inline fid hash(char c, string fstr) { 
    str[0] = c;
    return VW::hash_feature(*vw_ref, fstr, VW::hash_space(*vw_ref, str)); 
  }
  inline fid hash(char c, char* fstr) { 
    str[0] = c;
    return VW::hash_feature_cstr(*vw_ref, fstr, VW::hash_space(*vw_ref, str)); 
  }

  inline fid addf(fid    fint           ) { return addf(fint      , 1.0); }
  inline fid addf(string fstr, float val) { return addf(hash(fstr), val); }
  inline fid addf(string fstr           ) { return addf(hash(fstr), 1.0); }

  inline fid addf(char ns, fid    fint           ) { return addf(ns, fint      , 1.0); }
  inline fid addf(char ns, string fstr, float val) { return addf(ns, hash(fstr), val); }
  inline fid addf(char ns, string fstr           ) { return addf(ns, hash(fstr), 1.0); }

  inline ezexample_new& operator()(fid         fint           ) { addf(fint, 1.0); return *this; }
  inline ezexample_new& operator()(string      fstr           ) { addf(fstr, 1.0); return *this; }
  inline ezexample_new& operator()(const char* fstr           ) { addf(fstr, 1.0); return *this; }
  inline ezexample_new& operator()(fid         fint, float val) { addf(fint, val); return *this; }
  inline ezexample_new& operator()(string      fstr, float val) { addf(fstr, val); return *this; }
  inline ezexample_new& operator()(const char* fstr, float val) { addf(fstr, val); return *this; }
  inline ezexample_new& operator()(const vw_namespace&n) { addns(n.namespace_letter); return *this; }
  inline ezexample_new& operator--() { remns(); return *this; }
  inline float      operator()() { return predict(); }
};


#endif

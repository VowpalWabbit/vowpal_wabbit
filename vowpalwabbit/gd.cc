/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include "crossplat_compat.h"

#include <float.h>
#ifdef _WIN32
#include <WinSock2.h>
#else
#include <netdb.h>
#endif

#if !defined(VW_NO_INLINE_SIMD)
#  if defined(__ARM_NEON__)
#include <arm_neon.h>
#  elif defined(__SSE2__)
#include <xmmintrin.h>
#  endif
#endif

#include "gd.h"
#include "accumulate.h"
#include "reductions.h"
#include "vw.h"
#include "floatbits.h"

#define VERSION_SAVE_RESUME_FIX "7.10.1"

using namespace std;
using namespace LEARNER;
//todo:
//4. Factor various state out of vw&
namespace GD
{
struct gd
{ //double normalized_sum_norm_x;
  double total_weight;
  size_t no_win_counter;
  size_t early_stop_thres;
  float initial_constant;
  float neg_norm_power;
  float neg_power_t;
  float sparse_l2;
  float update_multiplier;
  void (*predict)(gd&, base_learner&, example&);
  void (*learn)(gd&, base_learner&, example&);
  void (*update)(gd&, base_learner&, example&);
  float (*sensitivity)(gd&, base_learner&, example&);
  void (*multipredict)(gd&, base_learner&, example&, size_t, size_t, polyprediction*, bool);
  bool normalized;
  bool adaptive;

  vw* all; //parallel, features, parameters
};

void sync_weights(vw& all);

inline float quake_InvSqrt(float x)
{ // Carmack/Quake/SGI fast method:
  float xhalf = 0.5f * x;
  int i = float_to_bits(x); // store floating-point bits in integer
  i = 0x5f3759d5 - (i >> 1); // initial guess for Newton's method
  x = bits_to_float(i); // convert new bits into float
  x = x*(1.5f - xhalf*x*x); // One round of Newton's method
  return x;
}

static inline float InvSqrt(float x)
{
#if !defined(VW_NO_INLINE_SIMD)
#  if defined(__ARM_NEON__)
  // Propagate into vector
  float32x2_t v1 = vdup_n_f32(x);
  // Estimate
  float32x2_t e1 = vrsqrte_f32(v1);
  // N-R iteration 1
  float32x2_t e2 = vmul_f32(e1, vrsqrts_f32(v1, vmul_f32(e1, e1)));
  // N-R iteration 2
  float32x2_t e3 = vmul_f32(e2, vrsqrts_f32(v1, vmul_f32(e2, e2)));
  // Extract result
  return vget_lane_f32(e3, 0);
#  elif (defined(__SSE2__) || defined(_M_AMD64) || defined(_M_X64))
  __m128 eta = _mm_load_ss(&x);
  eta = _mm_rsqrt_ss(eta);
  _mm_store_ss(&x, eta);
#else
  x = quake_InvSqrt(x);
#  endif
#else
  x = quake_InvSqrt(x);
#endif

  return x;
}

template<bool sqrt_rate, bool feature_mask_off, size_t adaptive, size_t normalized, size_t spare>
inline void update_feature(float& update, float x, float& fw)
{ weight* w = &fw;
  if(feature_mask_off || fw != 0.)
  { if (spare != 0)
      x *= w[spare];
    w[0] += update * x;
  }
}

//this deals with few nonzero features vs. all nonzero features issues.
template<bool sqrt_rate, size_t adaptive, size_t normalized>
float average_update(gd& g)
{ if (normalized)
  { if (sqrt_rate)
    { float avg_norm = (float) g.total_weight / (float) g.all->normalized_sum_norm_x;
      if (adaptive)
        return sqrt(avg_norm);
      else
        return avg_norm;
    }
    else
      return powf( (float) g.all->normalized_sum_norm_x / (float) g.total_weight, g.neg_norm_power);
  }
  return 1.f;
}

template<bool sqrt_rate, bool feature_mask_off, size_t adaptive, size_t normalized, size_t spare>
void train(gd& g, example& ec, float update)
{ if (normalized)
    update *= g.update_multiplier;

  foreach_feature<float, update_feature<sqrt_rate, feature_mask_off, adaptive, normalized, spare> >(*g.all, ec, update);
}

void end_pass(gd& g)
{ vw& all = *g.all;
  sync_weights(all);
  if (all.all_reduce != nullptr)
  { if (all.adaptive)
      accumulate_weighted_avg(all, all.reg);
    else
      accumulate_avg(all, all.reg, 0);
  }
  all.eta *= all.eta_decay_rate;
  if (all.save_per_pass)
    save_predictor(all, all.final_regressor_name, all.current_pass);

  all.current_pass++;

  if(!all.holdout_set_off)
  { if(summarize_holdout_set(all, g.no_win_counter))
      finalize_regressor(all, all.final_regressor_name);
    if((g.early_stop_thres == g.no_win_counter) &&
        ((all.check_holdout_every_n_passes <= 1) ||
         ((all.current_pass % all.check_holdout_every_n_passes) == 0)))
      set_done(all);
  }
}

#include <algorithm>

struct string_value
{ float v;
  string s;
  friend bool operator<(const string_value& first, const string_value& second);
};

inline float sign(float w) { if (w < 0.) return -1.; else  return 1.;}

inline float trunc_weight(const float w, const float gravity)
{ return (gravity < fabsf(w)) ? w - sign(w) * gravity : 0.f;
}

bool operator<(const string_value& first, const string_value& second)
{ return fabsf(first.v) > fabsf(second.v);
}

struct audit_results
{ vw& all;
  const uint64_t offset;
  vector<string> ns_pre;
  vector<string_value> results;
  audit_results(vw& p_all, const size_t p_offset):all(p_all), offset(p_offset) {}
};


inline void audit_interaction(audit_results& dat, const audit_strings* f)
{ if (f == nullptr)
  { dat.ns_pre.pop_back();
    return;
  }

  string ns_pre;
  if (!dat.ns_pre.empty())
    ns_pre += '*';

  if (f->first != "" && ((f->first) != " "))
    {
      ns_pre.append(f->first);
      ns_pre += '^';
    }
  if (f->second != "")
    {
      ns_pre.append(f->second);
      dat.ns_pre.push_back(ns_pre);
    }
}

inline void audit_feature(audit_results& dat, const float ft_weight, const uint64_t ft_idx)
{ uint64_t index = ft_idx & dat.all.reg.weight_mask;
  weight* weights = dat.all.reg.weight_vector;
  size_t stride_shift = dat.all.reg.stride_shift;

  string ns_pre;
  for (string& s : dat.ns_pre) ns_pre += s;

  if(dat.all.audit)
  { ostringstream tempstream;
    tempstream << ':' << (index >> stride_shift) << ':' << ft_weight
               << ':' << trunc_weight(weights[index], (float)dat.all.sd->gravity) * (float)dat.all.sd->contraction;

    if(dat.all.adaptive)
      tempstream << '@' << weights[index+1];


    string_value sv = {weights[index]*ft_weight, ns_pre+tempstream.str()};
    dat.results.push_back(sv);
  }

  if(dat.all.current_pass == 0 && dat.all.hash_inv)
  { //for invert_hash

    if (dat.offset != 0)
    { // otherwise --oaa output no features for class > 0.
      ostringstream tempstream;
      tempstream << '[' << (dat.offset >> stride_shift) << ']';
      ns_pre += tempstream.str();
    }

    if(!dat.all.name_index_map.count(ns_pre))
      dat.all.name_index_map.insert(std::map< std::string, size_t>::value_type(ns_pre, index >> stride_shift));
  }

}

void print_features(vw& all, example& ec)
{ weight* weights = all.reg.weight_vector;

  if (all.lda > 0)
  { size_t count = 0;
    for (features& fs : ec)
      count += fs.size();
    for (features& fs : ec)
    { for (features::iterator_all& f : fs.values_indices_audit())
      { cout << '\t' << f.audit().get()->first << '^' << f.audit().get()->second << ':' << ((f.index() >> all.reg.stride_shift) & all.parse_mask) << ':' << f.value();
        for (size_t k = 0; k < all.lda; k++)
          cout << ':' << weights[(f.index()+k) & all.reg.weight_mask];
      }
    }
    cout << " total of " << count << " features." << endl;
  }
  else
  {

    audit_results dat(all,ec.ft_offset);

    for (features& fs : ec)
    { if (fs.space_names.size() > 0)
        for (features::iterator_all& f : fs.values_indices_audit())
	      {
          audit_interaction(dat, f.audit().get());
	        audit_feature(dat, f.value(), f.index() + ec.ft_offset);
	        audit_interaction(dat, NULL);
	      }
	    else
        for (features::iterator& f : fs)
          audit_feature(dat, f.value(), f.index() + ec.ft_offset);
    }

    INTERACTIONS::generate_interactions<audit_results, const uint64_t, audit_feature, true, audit_interaction >(all, ec, dat);

    sort(dat.results.begin(),dat.results.end());
    if(all.audit)
    { for (string_value& sv : dat.results)
        cout << '\t' << sv.s;
      cout << endl;
    }

  }
}

void print_audit_features(vw& all, example& ec)
{ if(all.audit)
    print_result(all.stdout_fileno,ec.pred.scalar,-1,ec.tag);
  fflush(stdout);
  print_features(all, ec);
}

float finalize_prediction(shared_data* sd, float ret)
{ if ( nanpattern(ret))
  { float ret = 0.;
    if (ret > sd->max_label) ret = (float)sd->max_label;
    if (ret < sd->min_label) ret = (float)sd->min_label;
    cerr << "NAN prediction in example " << sd->example_number + 1 << ", forcing " << ret << endl;
    return ret;
  }
  if ( ret > sd->max_label )
    return (float)sd->max_label;
  if (ret < sd->min_label)
    return (float)sd->min_label;
  return ret;
}

struct trunc_data
{ float prediction;
  float gravity;
};

inline void vec_add_trunc(trunc_data& p, const float fx, float& fw)
{ p.prediction += trunc_weight(fw, p.gravity) * fx;
}

inline float trunc_predict(vw& all, example& ec, double gravity)
{ trunc_data temp = {ec.l.simple.initial, (float)gravity};
  foreach_feature<trunc_data, vec_add_trunc>(all, ec, temp);
  return temp.prediction;
}

inline void vec_add_print(float&p, const float fx, float& fw)
{ p += fw * fx;
  cerr << " + " << fw << "*" << fx;
}

template<bool l1, bool audit>
void predict(gd& g, base_learner&, example& ec)
{ vw& all = *g.all;

  if (l1)
    ec.partial_prediction = trunc_predict(all, ec, all.sd->gravity);
  else
    ec.partial_prediction = inline_predict(all, ec);

  ec.partial_prediction *= (float)all.sd->contraction;
  ec.pred.scalar = finalize_prediction(all.sd, ec.partial_prediction);
  if (audit)
    print_audit_features(all, ec);
}

inline void vec_add_trunc_multipredict(multipredict_info& mp, const float fx, uint64_t fi)
{ weight*w = mp.reg->weight_vector + (fi & mp.reg->weight_mask);
  for (size_t c=0; c<mp.count; c++)
  { mp.pred[c].scalar += fx * trunc_weight(*w, mp.gravity);
    w += mp.step;
  }
}

template<bool l1, bool audit>
void multipredict(gd& g, base_learner&, example& ec, size_t count, size_t step, polyprediction*pred, bool finalize_predictions)
{ vw& all = *g.all;
  for (size_t c=0; c<count; c++)
    pred[c].scalar = ec.l.simple.initial;
  multipredict_info mp = { count, step, pred, &g.all->reg, (float)all.sd->gravity };
  if (l1) foreach_feature<multipredict_info, uint64_t, vec_add_trunc_multipredict>(all, ec, mp);
  else    foreach_feature<multipredict_info, uint64_t, vec_add_multipredict      >(all, ec, mp);
  if (all.sd->contraction != 1.)
    for (size_t c=0; c<count; c++)
      pred[c].scalar *= (float)all.sd->contraction;
  if (finalize_predictions)
    for (size_t c=0; c<count; c++)
      pred[c].scalar = finalize_prediction(all.sd, pred[c].scalar);
  if (audit)
  { for (size_t c=0; c<count; c++)
    { ec.pred.scalar = pred[c].scalar;
      print_audit_features(all, ec);
      ec.ft_offset += (uint64_t)step;
    }
    ec.ft_offset -= (uint64_t)(step*count);
  }
}


struct power_data
{ float minus_power_t;
  float neg_norm_power;
};

template<bool sqrt_rate, size_t adaptive, size_t normalized>
inline float compute_rate_decay(power_data& s, float& fw)
{ weight* w = &fw;
  float rate_decay = 1.f;
  if(adaptive)
  { if (sqrt_rate)
    { rate_decay = InvSqrt(w[adaptive]);
    }
    else
      rate_decay = powf(w[adaptive],s.minus_power_t);
  }
  if(normalized)
  { if (sqrt_rate)
    { float inv_norm = 1.f / w[normalized];
      if (adaptive)
        rate_decay *= inv_norm;
      else
        rate_decay *= inv_norm*inv_norm;
    }
    else
      rate_decay *= powf(w[normalized]*w[normalized], s.neg_norm_power);
  }
  return rate_decay;
}

struct norm_data
{ float grad_squared;
  float pred_per_update;
  float norm_x;
  power_data pd;
};

const float x_min = 1.084202e-19f;
const float x2_min = x_min*x_min;

template<bool sqrt_rate, bool feature_mask_off, size_t adaptive, size_t normalized, size_t spare, bool stateless>
inline void pred_per_update_feature(norm_data& nd, float x, float& fw)
{ if(feature_mask_off || fw != 0.)
  { weight* w = &fw;
    float x2 = x * x;
    if (x2 < x2_min)
    { x = (x>0)? x_min:-x_min;
      x2 = x2_min;
    }
    if(adaptive && !stateless)
      w[adaptive] += nd.grad_squared * x2;
    if(normalized)
    { float x_abs = fabsf(x);
      if( x_abs > w[normalized] && !stateless)  // new scale discovered
      { if( w[normalized] > 0.)  //If the normalizer is > 0 then rescale the weight so it's as if the new scale was the old scale.
        { if (sqrt_rate)
          { float rescale = w[normalized]/x_abs;
            w[0] *= (adaptive ? rescale : rescale*rescale);
          }
          else
          { float rescale = x_abs/w[normalized];
            w[0] *= powf(rescale*rescale, nd.pd.neg_norm_power);
          }
        }
        w[normalized] = x_abs;
      }
      nd.norm_x += x2 / (w[normalized] * w[normalized]);
    }
    w[spare] = compute_rate_decay<sqrt_rate, adaptive, normalized>(nd.pd, fw);
    nd.pred_per_update += x2 * w[spare];
  }
}

bool global_print_features = false;
template<bool sqrt_rate, bool feature_mask_off, size_t adaptive, size_t normalized, size_t spare, bool stateless>
float get_pred_per_update(gd& g, example& ec)
{ //We must traverse the features in _precisely_ the same order as during training.
  label_data& ld = ec.l.simple;
  vw& all = *g.all;
  float grad_squared = all.loss->getSquareGrad(ec.pred.scalar, ld.label) * ec.weight;
  if (grad_squared == 0 && !stateless) return 1.;

  norm_data nd = {grad_squared, 0., 0., {g.neg_power_t, g.neg_norm_power}};

  foreach_feature<norm_data,pred_per_update_feature<sqrt_rate, feature_mask_off, adaptive, normalized, spare, stateless> >(all, ec, nd);

  if(normalized)
  { if(!stateless)
    { g.all->normalized_sum_norm_x += ec.weight * nd.norm_x;
      g.total_weight += ec.weight;
    }
    g.update_multiplier = average_update<sqrt_rate, adaptive, normalized>(g);
    nd.pred_per_update *= g.update_multiplier;
  }

  return nd.pred_per_update;
}

template<bool sqrt_rate, bool feature_mask_off, size_t adaptive, size_t normalized, size_t spare, bool stateless>
float sensitivity(gd& g, example& ec)
{ if(adaptive || normalized)
    return get_pred_per_update<sqrt_rate, feature_mask_off, adaptive, normalized, spare, stateless>(g,ec);
  else
    return ec.total_sum_feat_sq;
}

template<size_t adaptive>
float get_scale(gd& g, example& ec, float weight)
{ float update_scale = g.all->eta * weight;
  if(!adaptive)
  { float t = (float)(ec.example_t - g.all->sd->weighted_holdout_examples);
    update_scale *= powf(t, g.neg_power_t);
  }
  return update_scale;
}

template<bool sqrt_rate, bool feature_mask_off, size_t adaptive, size_t normalized, size_t spare>
float sensitivity(gd& g, base_learner& base, example& ec)
{ return get_scale<adaptive>(g, ec, 1.)
         * sensitivity<sqrt_rate, feature_mask_off, adaptive, normalized, spare, true>(g,ec);
}

template<bool sparse_l2, bool invariant, bool sqrt_rate, bool feature_mask_off, size_t adaptive, size_t normalized, size_t spare>
float compute_update(gd& g, example& ec)
{ //invariant: not a test label, importance weight > 0
  label_data& ld = ec.l.simple;
  vw& all = *g.all;

  float update = 0.;
  ec.updated_prediction = ec.pred.scalar;
  if (all.loss->getLoss(all.sd, ec.pred.scalar, ld.label) > 0.)
  { float pred_per_update = sensitivity<sqrt_rate, feature_mask_off, adaptive, normalized, spare, false>(g, ec);
    float update_scale = get_scale<adaptive>(g, ec, ec.weight);

    if(invariant)
      update = all.loss->getUpdate(ec.pred.scalar, ld.label, update_scale, pred_per_update);
    else
      update = all.loss->getUnsafeUpdate(ec.pred.scalar, ld.label, update_scale);
    // changed from ec.partial_prediction to ld.prediction
    ec.updated_prediction += pred_per_update * update;

    if (all.reg_mode && fabs(update) > 1e-8)
    { double dev1 = all.loss->first_derivative(all.sd, ec.pred.scalar, ld.label);
      double eta_bar = (fabs(dev1) > 1e-8) ? (-update / dev1) : 0.0;
      if (fabs(dev1) > 1e-8)
        all.sd->contraction *= (1. - all.l2_lambda * eta_bar);
      update /= (float)all.sd->contraction;
      all.sd->gravity += eta_bar * all.l1_lambda;
    }
  }

  if (sparse_l2)
    update -= g.sparse_l2 * ec.pred.scalar;
  return update;
}

template<bool sparse_l2, bool invariant, bool sqrt_rate, bool feature_mask_off, size_t adaptive, size_t normalized, size_t spare>
void update(gd& g, base_learner&, example& ec)
{ //invariant: not a test label, importance weight > 0
  float update;
  if ( (update = compute_update<sparse_l2, invariant, sqrt_rate, feature_mask_off, adaptive, normalized, spare> (g, ec)) != 0.)
    train<sqrt_rate, feature_mask_off, adaptive, normalized, spare>(g, ec, update);

  if (g.all->sd->contraction < 1e-10)  // updating weights now to avoid numerical instability
    sync_weights(*g.all);
}

template<bool sparse_l2, bool invariant, bool sqrt_rate, bool feature_mask_off, size_t adaptive, size_t normalized, size_t spare>
void learn(gd& g, base_learner& base, example& ec)
{ //invariant: not a test label, importance weight > 0
  assert(ec.in_use);
  assert(ec.l.simple.label != FLT_MAX);
  assert(ec.weight > 0.);

  g.predict(g,base,ec);
  update<sparse_l2, invariant, sqrt_rate, feature_mask_off, adaptive, normalized, spare>(g,base,ec);
}

void sync_weights(vw& all)
{ if (all.sd->gravity == 0. && all.sd->contraction == 1.)  // to avoid unnecessary weight synchronization
    return;
  uint64_t length = (uint64_t)1 << all.num_bits;
  uint64_t stride = (uint64_t)1 << all.reg.stride_shift;
  for(uint64_t i = 0; i < length && all.reg_mode; i++)
    all.reg.weight_vector[stride*i] = trunc_weight(all.reg.weight_vector[stride*i], (float)all.sd->gravity) * (float)all.sd->contraction;
  all.sd->gravity = 0.;
  all.sd->contraction = 1.;
}

void save_load_regressor(vw& all, io_buf& model_file, bool read, bool text)
{ uint64_t length = (uint64_t)1 << all.num_bits;
  uint64_t stride = (uint64_t)1 << all.reg.stride_shift;
  uint64_t i = 0;
  uint32_t old_i = 0;
  size_t brw = 1;

  if(all.print_invert)   //write readable model with feature names
  { weight* v;
    stringstream msg;
    typedef std::map< std::string, size_t> str_int_map;

    for(str_int_map::iterator it = all.name_index_map.begin(); it != all.name_index_map.end(); ++it)
    { v = &(all.reg.weight_vector[stride*it->second]);
      if(*v != 0.)
      {
        msg << it->first;
        brw = bin_text_write_fixed(model_file, (char*)it->first.c_str(), sizeof(*it->first.c_str()),
                                   msg, true);

        msg << ":" << it->second << ":" << *v << "\n";
        bin_text_write_fixed(model_file, (char *)v, sizeof(*v), msg, true);
      }
    }
    return;
  }

  do
  { brw = 1;
    weight* v;
    if (read)
      { if (all.num_bits < 31)//backwards compatible
	  { brw = bin_read_fixed(model_file, (char*)&old_i, sizeof(old_i), "");
	    i = old_i;
	  }
	else
	  brw = bin_read_fixed(model_file, (char*)&i, sizeof(i), "");
      if (brw > 0)
      { if (i >= length)
        { THROW("Model content is corrupted, weight vector index " << i << " must be less than total vector length " << length);
        }
        v = &(all.reg.weight_vector[stride*i]);
        brw += bin_read_fixed(model_file, (char*)v, sizeof(*v), "");
      }
    }
    else// write binary or text
    {

      v = &(all.reg.weight_vector[stride*i]);
      if (*v != 0.)
        { stringstream msg;
          msg << i;

          if (all.num_bits < 31)
            {
              old_i = (uint32_t)i;
              brw = bin_text_write_fixed(model_file, (char *)&old_i, sizeof(old_i), msg, text);
            }
          else
            brw = bin_text_write_fixed(model_file, (char *)&i, sizeof(i), msg, text);

          msg << ":"<< *v << "\n";
          brw += bin_text_write_fixed(model_file, (char *)v, sizeof(*v), msg, text);
        }
    }

    if (!read)
      i++;
  }
  while ((!read && i < length) || (read && brw >0));
}

void save_load_online_state(vw& all, io_buf& model_file, bool read, bool text, gd* g)
{ //vw& all = *g.all;
  stringstream msg;

  msg << "initial_t " << all.initial_t << "\n";
  bin_text_read_write_fixed(model_file, (char*)&all.initial_t, sizeof(all.initial_t),
                            "", read, msg, text);

  msg << "norm normalizer "<< all.normalized_sum_norm_x << "\n";
  bin_text_read_write_fixed(model_file, (char*)&all.normalized_sum_norm_x, sizeof(all.normalized_sum_norm_x),
                            "", read, msg, text);

  msg << "t "<< all.sd->t <<"\n";
  bin_text_read_write_fixed(model_file, (char*)&all.sd->t, sizeof(all.sd->t),
                            "", read, msg, text);

  msg << "sum_loss "<< all.sd->sum_loss << "\n";
  bin_text_read_write_fixed(model_file, (char*)&all.sd->sum_loss, sizeof(all.sd->sum_loss),
                            "", read, msg, text);

  msg << "sum_loss_since_last_dump "<< all.sd->sum_loss_since_last_dump <<"\n";
  bin_text_read_write_fixed(model_file, (char*)&all.sd->sum_loss_since_last_dump, sizeof(all.sd->sum_loss_since_last_dump),
                            "", read, msg, text);

  msg << "dump_interval "<< all.sd->dump_interval << "\n";
  bin_text_read_write_fixed(model_file, (char*)&all.sd->dump_interval, sizeof(all.sd->dump_interval),
                            "", read, msg, text);

  msg << "min_label " << all.sd->min_label << "\n";
  bin_text_read_write_fixed(model_file, (char*)&all.sd->min_label, sizeof(all.sd->min_label),
                            "", read, msg, text);

  msg << "max_label " << all.sd->max_label << "\n";
  bin_text_read_write_fixed(model_file, (char*)&all.sd->max_label, sizeof(all.sd->max_label),
                            "", read, msg, text);

  msg << "weighted_examples "<< all.sd->weighted_examples << "\n";
  bin_text_read_write_fixed(model_file, (char*)&all.sd->weighted_examples, sizeof(all.sd->weighted_examples),
                            "", read, msg, text);

  msg << "weighted_labels "<< all.sd->weighted_labels << "\n";
  bin_text_read_write_fixed(model_file, (char*)&all.sd->weighted_labels, sizeof(all.sd->weighted_labels),
                            "", read, msg, text);

  msg << "weighted_unlabeled_examples " << all.sd->weighted_unlabeled_examples << "\n";
  bin_text_read_write_fixed(model_file, (char*)&all.sd->weighted_unlabeled_examples, sizeof(all.sd->weighted_unlabeled_examples),
                            "", read, msg, text);

  msg << "example_number " << all.sd->example_number << "\n";
  bin_text_read_write_fixed(model_file, (char*)&all.sd->example_number, sizeof(all.sd->example_number),
                            "", read, msg, text);

  msg << "total_features "<< all.sd->total_features << "\n";
  bin_text_read_write_fixed(model_file, (char*)&all.sd->total_features, sizeof(all.sd->total_features),
                            "", read, msg, text);

  if (!read || all.model_file_ver >= VERSION_SAVE_RESUME_FIX)
  { // restore some data to allow --save_resume work more accurate

    // fix average loss
    double total_weight = 0.; //value holder as g* may be null
    if (!read && g != nullptr) total_weight = g->total_weight;
    msg << "gd::total_weight " << total_weight << "\n";
    bin_text_read_write_fixed(model_file, (char*)&total_weight, sizeof(total_weight),
                              "", read, msg, text);
    if (read && g != nullptr) g->total_weight = total_weight;

    // fix "loss since last" for first printed out example details
    msg << "sd::oec.weighted_examples "<< all.sd->old_weighted_examples << "\n";
    bin_text_read_write_fixed(model_file, (char*)&all.sd->old_weighted_examples, sizeof(all.sd->old_weighted_examples),
                              "", read, msg, text);

    // fix "number of examples per pass"
    msg << "current_pass "<< all.current_pass << "\n";
    bin_text_read_write_fixed(model_file, (char*)&all.current_pass, sizeof(all.current_pass),
                              "", read, msg, text);
  }

  if (!all.training) // reset various things so that we report test set performance properly
  { all.sd->sum_loss = 0;
    all.sd->sum_loss_since_last_dump = 0;
    all.sd->dump_interval = 1.;
    all.sd->weighted_examples = 0.;
    all.sd->weighted_labels = 0.;
    all.sd->weighted_unlabeled_examples = 0.;
    all.sd->example_number = 0;
    all.sd->total_features = 0;
  }

  uint64_t length = (uint64_t)1 << all.num_bits;
  uint64_t stride = (uint64_t)1 << all.reg.stride_shift;

  int c = 0;
  uint64_t i = 0;
  size_t brw = 1;
  do
  { brw = 1;
    weight* v;
    if (read)
    { c++;
      brw = bin_read_fixed(model_file, (char*)&i, sizeof(i), "");
      if (brw > 0)
      { if (i >= length)
        { THROW("Model content is corrupted, weight vector index " << i << " must be less than total vector length " << length);
        }

        v = &(all.reg.weight_vector[stride*i]);
        if (g == NULL || (! g->adaptive && ! g->normalized))
          brw += bin_read_fixed(model_file, (char*)v, sizeof(*v), "");
        else if ((g->adaptive && !g->normalized) || (!g->adaptive && g->normalized))
          brw += bin_read_fixed(model_file, (char*)v, sizeof(*v) * 2, "");
        else //adaptive and normalized
          brw += bin_read_fixed(model_file, (char*)v, sizeof(*v) * 3, "");
        /*        if (!all.training)
                  v[1] = v[2] = 0.;*/
      }
    }
    else // write binary or text
    { v = &(all.reg.weight_vector[stride*i]);
      if (*v != 0.)
      { c++;

        msg << i;
        brw = bin_text_write_fixed(model_file, (char *)&i, sizeof(i),
                                   msg, text);
        if (g == nullptr || (! g->adaptive && ! g->normalized))
          { msg << ":" << *v << "\n";
          brw += bin_text_write_fixed(model_file, (char *)v, sizeof(*v),
                                      msg, text);
        }
        else if ((g->adaptive && !g->normalized) || (!g->adaptive && g->normalized))
        { //either adaptive or normalized
          msg << ":"<< *v << " "<< *(v+1)<< "\n";
          brw+= bin_text_write_fixed(model_file,(char *)v, 2*sizeof (*v),
                                     msg, text);
        }
        else
          { //adaptive and normalized
            msg << ":"<< *v << " "<< *(v+1)<< " "<< *(v+2)<< "\n";
            brw+= bin_text_write_fixed(model_file,(char *)v, 3*sizeof (*v),
                                       msg, text);
        }
      }
    }
    if (!read)
      i++;
  }
  while ((!read && i < length) || (read && brw >0));
}

void save_load(gd& g, io_buf& model_file, bool read, bool text)
{ vw& all = *g.all;
  if(read)
  { initialize_regressor(all);

    if(all.adaptive && all.initial_t > 0)
      { uint64_t length = (uint64_t)1 << all.num_bits;
	uint64_t stride = (uint64_t)1 << all.reg.stride_shift;
      for (uint64_t j = 1; j < stride*length; j+=stride)
      { all.reg.weight_vector[j] = all.initial_t;   //for adaptive update, we interpret initial_t as previously seeing initial_t fake datapoints, all with squared gradient=1
        //NOTE: this is not invariant to the scaling of the data (i.e. when combined with normalized). Since scaling the data scales the gradient, this should ideally be
        //feature_range*initial_t, or something like that. We could potentially fix this by just adding this base quantity times the current range to the sum of gradients
        //stored in memory at each update, and always start sum of gradients to 0, at the price of additional additions and multiplications during the update...
      }
    }

    if (g.initial_constant != 0.0)
      VW::set_weight(all, constant, 0, g.initial_constant);
  }

  if (model_file.files.size() > 0)
  { bool resume = all.save_resume;
    stringstream msg;
    msg << ":"<< resume << "\n";
    bin_text_read_write_fixed(model_file,(char *)&resume, sizeof (resume),
                              "", read,
                              msg, text);
    if (resume)
    { if (read && all.model_file_ver < VERSION_SAVE_RESUME_FIX)
        cerr << endl << "WARNING: --save_resume functionality is known to have inaccuracy in model files version less than " << VERSION_SAVE_RESUME_FIX << endl << endl;
      // save_load_online_state(g, model_file, read, text);
      save_load_online_state(all, model_file, read, text, &g);
    }
    else
      save_load_regressor(all, model_file, read, text);
  }
}

template<bool sparse_l2, bool invariant, bool sqrt_rate, uint64_t adaptive, uint64_t normalized, uint64_t spare, uint64_t next>
uint64_t set_learn(vw& all, bool feature_mask_off, gd& g)
{ all.normalized_idx = normalized;
  if (feature_mask_off)
  { g.learn = learn<sparse_l2, invariant, sqrt_rate, true, adaptive, normalized, spare>;
    g.update = update<sparse_l2, invariant, sqrt_rate, true, adaptive, normalized, spare>;
    g.sensitivity = sensitivity<sqrt_rate, true, adaptive, normalized, spare>;
    return next;
  }
  else
  { g.learn = learn<sparse_l2, invariant, sqrt_rate, false, adaptive, normalized, spare>;
    g.update = update<sparse_l2, invariant, sqrt_rate, false, adaptive, normalized, spare>;
    g.sensitivity = sensitivity<sqrt_rate, false, adaptive, normalized, spare>;
    return next;
  }
}

template<bool invariant, bool sqrt_rate, uint64_t adaptive, uint64_t normalized, uint64_t spare, uint64_t next>
uint64_t set_learn(vw& all, bool feature_mask_off, gd& g)
{ if (g.sparse_l2 > 0.f)
    return set_learn<true, invariant, sqrt_rate, adaptive, normalized, spare, next>(all, feature_mask_off, g);
  else
    return set_learn<false, invariant, sqrt_rate, adaptive, normalized, spare, next>(all, feature_mask_off, g);
}

template<bool sqrt_rate, uint64_t adaptive, uint64_t normalized, uint64_t spare, uint64_t next>
uint64_t set_learn(vw& all, bool feature_mask_off, gd& g)
{ if (all.invariant_updates)
    return set_learn<true, sqrt_rate, adaptive, normalized, spare, next>(all, feature_mask_off, g);
  else
    return set_learn<false, sqrt_rate, adaptive, normalized, spare, next>(all, feature_mask_off, g);
}

template<bool sqrt_rate, uint64_t adaptive, uint64_t spare>
uint64_t set_learn(vw& all, bool feature_mask_off, gd& g)
{ // select the appropriate learn function based on adaptive, normalization, and feature mask
  if (all.normalized_updates)
    return set_learn<sqrt_rate, adaptive, adaptive+1, adaptive+2, adaptive+3>(all, feature_mask_off, g);
  else
    return set_learn<sqrt_rate, adaptive, 0, spare, spare+1>(all, feature_mask_off, g);
}

template<bool sqrt_rate>
uint64_t set_learn(vw& all, bool feature_mask_off, gd& g)
{ if (all.adaptive)
    return set_learn<sqrt_rate, 1, 2>(all, feature_mask_off, g);
  else
    return set_learn<sqrt_rate, 0, 0>(all, feature_mask_off, g);
}

uint64_t ceil_log_2(uint64_t v)
{ if (v==0)
    return 0;
  else
    return 1 + ceil_log_2(v >> 1);
}

base_learner* setup(vw& all)
{ new_options(all, "Gradient Descent options")
  ("sgd", "use regular stochastic gradient descent update.")
  ("adaptive", "use adaptive, individual learning rates.")
  ("invariant", "use safe/importance aware updates.")
  ("normalized", "use per feature normalized updates")
  ("sparse_l2", po::value<float>()->default_value(0.f), "use per feature normalized updates");
  add_options(all);
  po::variables_map& vm = all.vm;
  gd& g = calloc_or_throw<gd>();
  g.all = &all;
  g.all->normalized_sum_norm_x = 0;
  g.no_win_counter = 0;
  g.total_weight = 0.;
  g.early_stop_thres = 3;
  g.sparse_l2 = vm["sparse_l2"].as<float>();
  g.neg_norm_power = (all.adaptive ? (all.power_t - 1.f) : -1.f);
  g.neg_power_t = - all.power_t;
  g.adaptive = all.adaptive;
  g.normalized = all.normalized_updates;

  if(all.initial_t > 0)//for the normalized update: if initial_t is bigger than 1 we interpret this as if we had seen (all.initial_t) previous fake datapoints all with norm 1
  { g.all->normalized_sum_norm_x = all.initial_t;
    g.total_weight = all.initial_t;
  }

  bool feature_mask_off = true;
  if(vm.count("feature_mask"))
    feature_mask_off = false;

  if(!all.holdout_set_off)
  { all.sd->holdout_best_loss = FLT_MAX;
    if(vm.count("early_terminate"))
      g.early_stop_thres = vm["early_terminate"].as< size_t>();
  }

  if (vm.count("constant"))
  { g.initial_constant = vm["constant"].as<float>();
  }

  if( vm.count("sgd") || vm.count("adaptive") || vm.count("invariant") || vm.count("normalized") )
  { //nondefault
    all.adaptive = all.training && vm.count("adaptive");
    all.invariant_updates = all.training && vm.count("invariant");
    all.normalized_updates = all.training && vm.count("normalized");

    if(!vm.count("learning_rate") && !vm.count("l") && !(all.adaptive && all.normalized_updates))
      all.eta = 10; //default learning rate to 10 for non default update rule

    //if not using normalized or adaptive, default initial_t to 1 instead of 0
    if(!all.adaptive && !all.normalized_updates)
    { if (!vm.count("initial_t"))
      { all.sd->t = 1.f;
        all.sd->weighted_unlabeled_examples = 1.f;
        all.initial_t = 1.f;
      }
      all.eta *= powf((float)(all.sd->t), all.power_t);
    }
  }

  if (pow((double)all.eta_decay_rate, (double)all.numpasses) < 0.0001 )
    cerr << "Warning: the learning rate for the last pass is multiplied by: " << pow((double)all.eta_decay_rate, (double)all.numpasses)
         << " adjust --decay_learning_rate larger to avoid this." << endl;



  if (all.reg_mode % 2)
    if (all.audit || all.hash_inv)
    { g.predict = predict<true, true>;   g.multipredict = multipredict<true, true>;
    }
    else
    { g.predict = predict<true, false>;  g.multipredict = multipredict<true, false>;
    }
  else if (all.audit || all.hash_inv)
  { g.predict = predict<false, true>;    g.multipredict = multipredict<false, true>;
  }
  else
  { g.predict = predict<false, false>;   g.multipredict = multipredict<false, false>;
  }

  uint64_t stride;
  if (all.power_t == 0.5)
    stride = set_learn<true>(all, feature_mask_off, g);
  else
    stride = set_learn<false>(all, feature_mask_off, g);
  if (!all.training)
    stride = 1;
  all.reg.stride_shift = (uint32_t)ceil_log_2(stride-1);

  learner<gd>& ret = init_learner(&g, g.learn, ((uint64_t)1 << all.reg.stride_shift));
  ret.set_predict(g.predict);
  ret.set_sensitivity(g.sensitivity);
  ret.set_multipredict(g.multipredict);
  ret.set_update(g.update);
  ret.set_save_load(save_load);
  ret.set_end_pass(end_pass);
  return make_base(ret);
}
}

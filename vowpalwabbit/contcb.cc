// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <random>

#include "gd.h"
#include "io_buf.h"
#include "parse_regressor.h"
#include "contcb.h"
#include "vw.h"
#include "vw_math.h"

using namespace std;
using namespace VW::LEARNER;
using namespace VW::config;

namespace VW
{
namespace continuous_cb
{
constexpr uint8_t tmodel_const = 0;
constexpr uint8_t tmodel_lin = 1;

struct contcb
{
  float radius;
  vw* all;
  bool min_prediction_supplied, max_prediction_supplied;
};

struct linear_update_data
{
  float mult;
  float part_grad;
  vw* all;
};

// uint64_t index variant of VW::get_weight
inline float get_weight(vw& all, uint64_t index, uint32_t offset)
{
  return (&all.weights[(index) << all.weights.stride_shift()])[offset];
}

// uint64_t index variant of VW::set_weight
inline void set_weight(vw& all, uint64_t index, uint32_t offset, float value)
{
  (&all.weights[(index) << all.weights.stride_shift()])[offset] = value;
}

// Checks if dir is a unit scalar with some tolerance. If yes, returns
// true and optionally fixes that tol difference.
inline bool check_fix_unit(float& dir, float tol = 0.01f, bool fix = true)
{
  if (!(VW::math::are_same(dir, 1.0f, tol) || VW::math::are_same(dir, -1.0f, tol))) return false;
  if (fix) dir = dir >= 0 ? 1.0f : -1.0f;
  return true;
}

float l1_grad(vw& all, uint64_t fi)
{
  if (all.no_bias && fi == constant) return 0.0f;

  float fw = get_weight(all, fi, 0);
  return fw >= 0.0f ? all.l1_lambda : -all.l1_lambda;
}

float l2_grad(vw& all, uint64_t fi)
{
  if (all.no_bias && fi == constant) return 0.0f;

  float fw = get_weight(all, fi, 0);
  return all.l2_lambda * fw;
}

inline void accumulate_dotprod(float& dotprod, float x, float& fw) { dotprod += x * fw; }

inline float constant_inference(vw& all)
{
  float wt = get_weight(all, constant, 0);
  return wt;
}

float linear_inference(vw& all, example& ec)
{
  float dotprod = 0;
  GD::foreach_feature<float, accumulate_dotprod>(all, ec, dotprod);
  return dotprod;
}

template <uint8_t tmodel>
float inference(vw& all, example& ec)
{
  if (tmodel == tmodel_const)
    return constant_inference(all);

  else if (tmodel == tmodel_lin)
    return linear_inference(all, ec);

  else
    THROW("Unknown template model encountered: " << tmodel);
}

template <uint8_t tmodel>
inline float compute_explore_dir(contcb& data, example& ec)
{
  return (ec.l.cb_cont.costs[0].action - inference<tmodel>(*data.all, ec)) / data.radius;
}

template <bool feature_mask_off>
void constant_update(contcb& data, example& ec)
{
  float dir = compute_explore_dir<tmodel_const>(data, ec);
  if (!check_fix_unit(dir))
  {
    // The action is not part of the set of actions that was suggested to be explored by the
    // latest predict() and thus can't be used to learn.
    data.all->trace_message << "encountered example that does not help in learning" << std::endl;
    return;
  }

  float fw = get_weight(*data.all, constant, 0);
  if (feature_mask_off || fw != 0.0f)
  {
    float grad = (1 / data.radius) * ec.l.cb_cont.costs[0].cost * dir;
    float update = -data.all->eta * (grad + l1_grad(*data.all, constant) + l2_grad(*data.all, constant));

    set_weight(*data.all, constant, 0, fw + update);
  }
}

template <bool feature_mask_off>
void linear_per_feature_update(linear_update_data& upd_data, float x, uint64_t fi)
{
  float fw = get_weight(*upd_data.all, fi, 0);

  if (feature_mask_off || fw != 0.0f)
  {
    float update = upd_data.mult * (upd_data.part_grad * x + l1_grad(*upd_data.all, fi) + l2_grad(*upd_data.all, fi));
    set_weight(*upd_data.all, fi, 0, fw + update);
  }
}

template <bool feature_mask_off>
void linear_update(contcb& data, example& ec)
{
  float dir = compute_explore_dir<tmodel_lin>(data, ec);
  if (!check_fix_unit(dir))
  {
    // The action is not part of the set of actions that was suggested - to be explored - by the
    // latest predict() and thus can't be used to learn.
    data.all->trace_message << "encountered example that does not help in learning" << std::endl;
    return;
  }

  float mult = -data.all->eta;
  float part_grad = (1 / data.radius) * ec.l.cb_cont.costs[0].cost * dir;

  linear_update_data upd_data;
  upd_data.mult = mult;
  upd_data.part_grad = part_grad;
  upd_data.all = data.all;

  GD::foreach_feature<linear_update_data, uint64_t, linear_per_feature_update<feature_mask_off>>(
      *data.all, ec, upd_data);
}

template <uint8_t tmodel, bool feature_mask_off>
void update_weights(contcb& data, example& ec)
{
  if (tmodel == tmodel_const)
    constant_update<feature_mask_off>(data, ec);

  else if (tmodel == tmodel_lin)
    linear_update<feature_mask_off>(data, ec);

  else
    THROW("Unknown template model encountered: " << tmodel)
}

void set_minmax(shared_data *sd, float label, bool min_fixed, bool max_fixed)
{
  if (!min_fixed) sd->min_label = std::min(label, sd->min_label);
  if (!max_fixed) sd->max_label = std::max(label, sd->max_label);
}

std::string get_pred_repr(example& ec)
{
  std::stringstream ss;
  ss << ec.pred.scalars[0] << "," << ec.pred.scalars[1];  // <action_centroid>,<radius>
  return ss.str();
}

void print_audit_features(vw& all, example& ec)
{
  if (all.audit) all.print_text_by_ref(all.stdout_adapter.get(), get_pred_repr(ec), ec.tag);
  fflush(stdout);
  // Note: print_features() declaration was brought to gd.h so it can be used here. If it's
  // no longer used here, consider removing the declaration from gd.h.
  GD::print_features(all, ec);
}

template <uint8_t tmodel, bool audit_or_hash_inv>
void predict(contcb& data, base_learner&, example& ec)
{
  ec.pred.scalars.clear();

  float action_centroid = inference<tmodel>(*data.all, ec);
  set_minmax(data.all->sd, action_centroid, data.min_prediction_supplied, data.max_prediction_supplied);
  float clipped_action_centroid = std::min(std::max(action_centroid, data.all->sd->min_label), data.all->sd->max_label);

  ec.pred.scalars.push_back(clipped_action_centroid);
  ec.pred.scalars.push_back(data.radius);

  if (audit_or_hash_inv) print_audit_features(*data.all, ec);
}

template <uint8_t tmodel, bool feature_mask_off, bool audit_or_hash_inv>
void learn(contcb& data, base_learner& base, example& ec)
{
  // update_weights() doesn't require predict() to be called. It is called
  // to respect --audit, --invert_hash, --predictions for train examples
  predict<tmodel, audit_or_hash_inv>(data, base, ec);
  update_weights<tmodel, feature_mask_off>(data, ec);
}

inline void save_load_regressor(vw& all, io_buf& model_file, bool read, bool text)
{
  GD::save_load_regressor(all, model_file, read, text);
}

void save_load(contcb& data, io_buf& model_file, bool read, bool text)
{
  vw& all = *data.all;
  if (read)
  {
    initialize_regressor(all);
    if (data.all->initial_constant != 0.0f) set_weight(all, constant, 0, data.all->initial_constant);
  }
  if (model_file.num_files() > 0) save_load_regressor(all, model_file, read, text);
}

void output_prediction(vw& all, example& ec)
{
  std::string pred_repr = get_pred_repr(ec);
  for (auto& sink : all.final_prediction_sink) all.print_text_by_ref(sink.get(), pred_repr, ec.tag);
}

void finish_example(vw& all, contcb&, example& ec)
{
  output_prediction(all, ec);
  VW::finish_example(all, ec);
}

void (*get_learn(vw& all, uint8_t tmodel, bool feature_mask_off))(contcb&, base_learner&, example&)
{
  if (tmodel == tmodel_const)
    if (feature_mask_off)
      if (all.audit || all.hash_inv)
        return learn<tmodel_const, true, true>;
      else
        return learn<tmodel_const, true, false>;

    else if (all.audit || all.hash_inv)
      return learn<tmodel_const, false, true>;
    else
      return learn<tmodel_const, false, false>;

  else if (tmodel == tmodel_lin)
    if (feature_mask_off)
      if (all.audit || all.hash_inv)
        return learn<tmodel_lin, true, true>;
      else
        return learn<tmodel_lin, true, false>;

    else if (all.audit || all.hash_inv)
      return learn<tmodel_lin, false, true>;
    else
      return learn<tmodel_lin, false, false>;

  else
    THROW("Unknown template model encountered: " << tmodel)
}

void (*get_predict(vw& all, uint8_t tmodel))(contcb&, base_learner&, example&)
{
  if (tmodel == tmodel_const)
    if (all.audit || all.hash_inv)
      return predict<tmodel_const, true>;
    else
      return predict<tmodel_const, false>;

  else if (tmodel == tmodel_lin)
    if (all.audit || all.hash_inv)
      return predict<tmodel_lin, true>;
    else
      return predict<tmodel_lin, false>;

  else
    THROW("Unknown template model encountered: " << tmodel)
}

base_learner* setup(options_i& options, vw& all)
{
  auto data = scoped_calloc_or_throw<contcb>();

  std::string tmodel_str;
  bool contcb_option = false;

  option_group_definition new_options("Continuous Contextual Bandit Options");
  new_options
      .add(make_option("contcb", contcb_option)
               .keep()
               .necessary()
               .help("Solve 1-slot Continuous Action Contextual Bandit"))
      .add(make_option("template_model", tmodel_str).default_value("linear").keep().help("Template Model to Learn"))
      .add(make_option("radius", data->radius).default_value(0.1f).keep(all.save_resume).help("Exploration Radius"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  bool feature_mask_off = true;
  if (options.was_supplied("feature_mask")) feature_mask_off = false;

  uint8_t tmodel;
  if (tmodel_str.compare("constant") == 0)
    tmodel = tmodel_const;
  else if (tmodel_str.compare("linear") == 0)
    tmodel = tmodel_lin;
  else
    THROW("template_model must be in {'constant', 'linear'}");

  if (tmodel == tmodel_const)
  {
    if (options.was_supplied("noconstant")) THROW("constant template model can't be learnt when --noconstant is used")

    if (!feature_mask_off)
      all.trace_message
          << "warning: feature_mask used with constant template model (where there is only one weight to learn)."
          << std::endl;
  }

  all.p->lp = cb_continuous::the_label_parser;
  all.delete_prediction = delete_scalars;
  data->all = &all;
  data->min_prediction_supplied = options.was_supplied("min_prediction");
  data->max_prediction_supplied = options.was_supplied("max_prediction");

  learner<contcb, example>& l = init_learner(
      data, get_learn(all, tmodel, feature_mask_off), get_predict(all, tmodel), 0, prediction_type_t::scalars);

  l.set_save_load(save_load);
  l.set_finish_example(finish_example);

  return make_base(l);
}

}  // namespace continuous_cb
}  // namespace VW
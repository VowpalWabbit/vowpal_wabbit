// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#ifdef _WIN32
#define NOMINMAX
#include <winsock2.h>
#else
#include <netdb.h>
#endif

#include "reductions.h"
#include "gd.h"

using namespace LEARNER;
using namespace VW::config;

struct mf
{
  std::vector<std::string> pairs;

  size_t rank;

  uint32_t increment;

  // array to cache w*x, (l^k * x_l) and (r^k * x_r)
  // [ w*(1,x_l,x_r) , l^1*x_l, r^1*x_r, l^2*x_l, r^2*x_2, ... ]
  v_array<float> sub_predictions;

  // array for temp storage of indices during prediction
  v_array<unsigned char> predict_indices;

  // array for temp storage of indices
  v_array<unsigned char> indices;

  // array for temp storage of features
  features temp_features;

  vw* all;  // for pairs? and finalize

  ~mf()
  {
    // clean up local v_arrays
    indices.delete_v();
    sub_predictions.delete_v();
  }
};

template <bool cache_sub_predictions>
void predict(mf& data, single_learner& base, example& ec)
{
  float prediction = 0;
  if (cache_sub_predictions)
    data.sub_predictions.resize(2 * data.rank + 1);

  // predict from linear terms
  base.predict(ec);

  // store linear prediction
  if (cache_sub_predictions)
    data.sub_predictions[0] = ec.partial_prediction;
  prediction += ec.partial_prediction;

  // store namespace indices
  copy_array(data.predict_indices, ec.indices);

  // erase indices
  ec.indices.clear();
  ec.indices.push_back(0);

  // add interaction terms to prediction
  for (std::string& i : data.pairs)
  {
    int left_ns = (int)i[0];
    int right_ns = (int)i[1];

    if (ec.feature_space[left_ns].size() > 0 && ec.feature_space[right_ns].size() > 0)
    {
      for (size_t k = 1; k <= data.rank; k++)
      {
        ec.indices[0] = left_ns;

        // compute l^k * x_l using base learner
        base.predict(ec, k);
        float x_dot_l = ec.partial_prediction;
        if (cache_sub_predictions)
          data.sub_predictions[2 * k - 1] = x_dot_l;

        // set example to right namespace only
        ec.indices[0] = right_ns;

        // compute r^k * x_r using base learner
        base.predict(ec, k + data.rank);
        float x_dot_r = ec.partial_prediction;
        if (cache_sub_predictions)
          data.sub_predictions[2 * k] = x_dot_r;

        // accumulate prediction
        prediction += (x_dot_l * x_dot_r);
      }
    }
  }
  // restore namespace indices and label
  copy_array(ec.indices, data.predict_indices);

  // finalize prediction
  ec.partial_prediction = prediction;
  ec.pred.scalar = GD::finalize_prediction(data.all->sd, ec.partial_prediction);
}

void learn(mf& data, single_learner& base, example& ec)
{
  // predict with current weights
  predict<true>(data, base, ec);
  float predicted = ec.pred.scalar;

  // update linear weights
  base.update(ec);
  ec.pred.scalar = ec.updated_prediction;

  // store namespace indices
  copy_array(data.indices, ec.indices);

  // erase indices
  ec.indices.clear();
  ec.indices.push_back(0);

  // update interaction terms
  // looping over all pairs of non-empty namespaces
  for (std::string& i : data.pairs)
  {
    int left_ns = (int)i[0];
    int right_ns = (int)i[1];

    if (ec.feature_space[left_ns].size() > 0 && ec.feature_space[right_ns].size() > 0)
    {
      // set example to left namespace only
      ec.indices[0] = left_ns;

      // store feature values in left namespace
      data.temp_features.deep_copy_from(ec.feature_space[left_ns]);

      for (size_t k = 1; k <= data.rank; k++)
      {
        features& fs = ec.feature_space[left_ns];
        // multiply features in left namespace by r^k * x_r
        for (size_t i = 0; i < fs.size(); ++i) fs.values[i] *= data.sub_predictions[2 * k];

        // update l^k using base learner
        base.update(ec, k);

        // restore left namespace features (undoing multiply)
        fs.deep_copy_from(data.temp_features);

        // compute new l_k * x_l scaling factors
        // base.predict(ec, k);
        // data.sub_predictions[2*k-1] = ec.partial_prediction;
        // ec.pred.scalar = ec.updated_prediction;
      }

      // set example to right namespace only
      ec.indices[0] = right_ns;

      // store feature values for right namespace
      data.temp_features.deep_copy_from(ec.feature_space[right_ns]);

      for (size_t k = 1; k <= data.rank; k++)
      {
        features& fs = ec.feature_space[right_ns];
        // multiply features in right namespace by l^k * x_l
        for (size_t i = 0; i < fs.size(); ++i) fs.values[i] *= data.sub_predictions[2 * k - 1];

        // update r^k using base learner
        base.update(ec, k + data.rank);
        ec.pred.scalar = ec.updated_prediction;

        // restore right namespace features
        fs.deep_copy_from(data.temp_features);
      }
    }
  }
  // restore namespace indices
  copy_array(ec.indices, data.indices);

  // restore original prediction
  ec.pred.scalar = predicted;
}

void finish(mf& o)
{
  // restore global pairs
  o.all->pairs = o.pairs;
}

base_learner* mf_setup(options_i& options, vw& all)
{
  auto data = scoped_calloc_or_throw<mf>();
  option_group_definition new_options("Matrix Factorization Reduction");
  new_options.add(make_option("new_mf", data->rank).keep().help("rank for reduction-based matrix factorization"));
  options.add_and_parse(new_options);

  if (!options.was_supplied("new_mf"))
    return nullptr;

  data->all = &all;
  // store global pairs in local data structure and clear global pairs
  // for eventual calls to base learner
  data->pairs = all.pairs;
  all.pairs.clear();

  all.random_positive_weights = true;

  learner<mf, example>& l =
      init_learner(data, as_singleline(setup_base(options, all)), learn, predict<false>, 2 * data->rank + 1);
  l.set_finish(finish);
  return make_base(l);
}

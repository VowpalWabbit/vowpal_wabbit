/*
 Copyright (c) by respective owners including Yahoo!, Microsoft, and
 individual contributors. All rights reserved.  Released under a BSD (revised)
 license as described in the file LICENSE.
 */
#ifdef _WIN32
#include <winsock2.h>
#else
#include <netdb.h>
#endif
#include "reductions.h"
#include "simple_label.h"
#include "gd.h"
#include "rand48.h"

using namespace std;

using namespace LEARNER;

namespace MF {

struct mf {
  vector<string> pairs;

  uint32_t rank;

  uint32_t increment;

  // array to cache w*x, (l^k * x_l) and (r^k * x_r)
  // [ w*(1,x_l,x_r) , l^1*x_l, r^1*x_r, l^2*x_l, r^2*x_2, ... ]
  v_array<float> sub_predictions;

  // array for temp storage of indices during prediction
  v_array<unsigned char> predict_indices;

  // array for temp storage of indices
  v_array<unsigned char> indices;

  // array for temp storage of features
  v_array<feature> temp_features;

  vw* all;
};

template <bool cache_sub_predictions>
void predict(mf& data, learner& base, example& ec) {
  float prediction = 0;
  if (cache_sub_predictions)
    data.sub_predictions.resize(2*data.rank+1, true);

  // predict from linear terms
  base.predict(ec);

  // store linear prediction
  if (cache_sub_predictions)
    data.sub_predictions[0] = ec.partial_prediction;
  prediction += ec.partial_prediction;

  // store namespace indices
  copy_array(data.predict_indices, ec.indices);

  // erase indices
  ec.indices.erase();
  ec.indices.push_back(0);

  // add interaction terms to prediction
  for (vector<string>::iterator i = data.pairs.begin(); i != data.pairs.end(); i++) {

    int left_ns = (int) (*i)[0];
    int right_ns = (int) (*i)[1];

    if (ec.atomics[left_ns].size() > 0 && ec.atomics[right_ns].size() > 0) {
      for (size_t k = 1; k <= data.rank; k++) {

	ec.indices[0] = left_ns;

	// compute l^k * x_l using base learner
	base.predict(ec, k);
	float x_dot_l = ec.partial_prediction;
	if (cache_sub_predictions)
	  data.sub_predictions[2*k-1] = x_dot_l;

	// set example to right namespace only
	ec.indices[0] = right_ns;

	// compute r^k * x_r using base learner
	base.predict(ec, k + data.rank);
	float x_dot_r = ec.partial_prediction;
	if (cache_sub_predictions)
	  data.sub_predictions[2*k] = x_dot_r;

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

void learn(mf& data, learner& base, example& ec) {
  // predict with current weights
  predict<true>(data, base, ec);
  float predicted = ec.pred.scalar;

  // update linear weights
  base.update(ec);
  ec.pred.scalar = ec.updated_prediction;

  // store namespace indices
  copy_array(data.indices, ec.indices);

  // erase indices
  ec.indices.erase();
  ec.indices.push_back(0);

  // update interaction terms
  // looping over all pairs of non-empty namespaces
  for (vector<string>::iterator i = data.pairs.begin(); i != data.pairs.end(); i++) {

    int left_ns = (int) (*i)[0];
    int right_ns = (int) (*i)[1];

    if (ec.atomics[left_ns].size() > 0 && ec.atomics[right_ns].size() > 0) {

      // set example to left namespace only
      ec.indices[0] = left_ns;

      // store feature values in left namespace
      copy_array(data.temp_features, ec.atomics[left_ns]);

      for (size_t k = 1; k <= data.rank; k++) {

	// multiply features in left namespace by r^k * x_r
	for (feature* f = ec.atomics[left_ns].begin; f != ec.atomics[left_ns].end; f++)
	  f->x *= data.sub_predictions[2*k];

	// update l^k using base learner
	base.update(ec, k);

	// restore left namespace features (undoing multiply)
	copy_array(ec.atomics[left_ns], data.temp_features);

	// compute new l_k * x_l scaling factors
	// base.predict(ec, k);
	// data.sub_predictions[2*k-1] = ec.partial_prediction;
	// ec.pred.scalar = ec.updated_prediction;
      }

      // set example to right namespace only
      ec.indices[0] = right_ns;

      // store feature values for right namespace
      copy_array(data.temp_features, ec.atomics[right_ns]);

      for (size_t k = 1; k <= data.rank; k++) {

	// multiply features in right namespace by l^k * x_l
	for (feature* f = ec.atomics[right_ns].begin; f != ec.atomics[right_ns].end; f++)
	  f->x *= data.sub_predictions[2*k-1];

	// update r^k using base learner
	base.update(ec, k + data.rank);
	ec.pred.scalar = ec.updated_prediction;

	// restore right namespace features
	copy_array(ec.atomics[right_ns], data.temp_features);
      }
    }
  }
  // restore namespace indices
  copy_array(ec.indices, data.indices);

  // restore original prediction
  ec.pred.scalar = predicted;
}

void finish(mf& o) {
  // restore global pairs
  o.all->pairs = o.pairs;

  // clean up local v_arrays
  o.indices.delete_v();
  o.sub_predictions.delete_v();
}


learner* setup(vw& all, po::variables_map& vm) {
  mf* data = new mf;

  // copy global data locally
  data->all = &all;
  data->rank = (uint32_t)vm["new_mf"].as<size_t>();

  // store global pairs in local data structure and clear global pairs
  // for eventual calls to base learner
  data->pairs = all.pairs;
  all.pairs.clear();

  all.random_positive_weights = true;

  learner* l = new learner(data, all.l, 2*data->rank+1);
  l->set_learn<mf, learn>();
  l->set_predict<mf, predict<false> >();
  l->set_finish<mf,finish>();
  return l;
}
}

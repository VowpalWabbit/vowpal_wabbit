/*
 Copyright (c) by respective owners including Yahoo!, Microsoft, and
 individual contributors. All rights reserved.  Released under a BSD (revised)
 license as described in the file LICENSE.
 */
#include <fstream>
#include <float.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <netdb.h>
#endif
#include <string.h>
#include <stdio.h>
#include <map>
#include "parse_example.h"
#include "constant.h"
#include "sparse_dense.h"
#include "gd.h"
#include "cache.h"
#include "simple_label.h"
#include "rand48.h"
#include "vw.h"
#include <algorithm>
#include "hash.h"
#include <sstream>
#include "parse_primitives.h"

using namespace std;

namespace MF {

struct mf {
  vector<string> pairs;

  uint32_t rank;

  uint32_t increment;

  // array to cache w*x, (l^k * x_l) and (r^k * x_r)
  // [ w*(1,x_l,x_r) , l^1*x_l, r^1*x_r, l^2*x_l, r^2*x_2, ... ]
  v_array<float> sub_predictions;

  // array for temp storage of indices
  v_array<unsigned char> indices;

  // array for temp storage of features
  v_array<feature> temp_features;

  vw* all;
};

void inline_predict(mf* data, vw* all, learner& base, example* &ec) {

  float prediction = 0;
  data->sub_predictions.resize(2*all->rank+1, true);

  // set weight to 0 to indicate test example (predict only)
  float weight = ((label_data*) ec->ld)->weight;
  ((label_data*) ec->ld)->weight = 0;

  // predict from linear terms
  base.learn(ec);

  // store linear prediction
  data->sub_predictions[0] = ec->partial_prediction;
  prediction += ec->partial_prediction;

  // store namespace indices
  copy_array(data->indices, ec->indices);

  // add interaction terms to prediction
  for (vector<string>::iterator i = data->pairs.begin(); i != data->pairs.end(); i++) {
    if (ec->atomics[(int) (*i)[0]].size() > 0 && ec->atomics[(int) (*i)[1]].size() > 0) {

      // set example to left namespace only
      ec->indices.erase();
      ec->indices.push_back((int) (*i)[0]);

      for (size_t k = 1; k <= all->rank; k++) {
	// compute l^k * x_l using base learner
	base.learn(ec, k);
	data->sub_predictions[2*k-1] = ec->partial_prediction;
      }

      // set example to right namespace only
      ec->indices.erase();
      ec->indices.push_back((int) (*i)[1]);

      for (size_t k = 1; k <= all->rank; k++) {
	// compute r^k * x_r using base learner
	base.learn(ec, k + all->rank);
	data->sub_predictions[2*k] = ec->partial_prediction;
      }

      // accumulate prediction
      for (size_t k = 1; k <= all->rank; k++)
	prediction += (data->sub_predictions[2*k-1] * data->sub_predictions[2*k]);
    }
  }
  // restore namespace indices and label
  copy_array(ec->indices, data->indices);

  ((label_data*) ec->ld)->weight = weight;

  // finalize prediction
  ec->partial_prediction = prediction;
  ec->final_prediction = GD::finalize_prediction(*(data->all), ec->partial_prediction);

}


void learn(void* d, learner& base, example* ec) {
  mf* data = (mf*) d;
  vw* all = data->all;

  // predict with current weights
  inline_predict(data, all, base, ec);

  // force base learner to use precomputed prediction
  ec->precomputed_prediction = true;

  // update linear weights
  base.learn(ec);

  // store namespace indices
  copy_array(data->indices, ec->indices);

  // update interaction terms
  // looping over all pairs of non-empty namespaces
  for (vector<string>::iterator i = data->pairs.begin(); i != data->pairs.end(); i++) {
    if (ec->atomics[(int) (*i)[0]].size() > 0 && ec->atomics[(int) (*i)[1]].size() > 0) {

      // set example to left namespace only
      ec->indices.erase();
      ec->indices.push_back((int) (*i)[0]);

      // store feature values in left namespace
      copy_array(data->temp_features, ec->atomics[(int) (*i)[0]]);

      for (size_t k = 1; k <= all->rank; k++) {

	// multiply features in left namespace by r^k * x_r
	for (feature* f = ec->atomics[(int) (*i)[0]].begin; f != ec->atomics[(int) (*i)[0]].end; f++)
	  f->x *= data->sub_predictions[2*k];

	// update l^k using base learner
	base.learn(ec, k);

	// restore left namespace features (undoing multiply)
	copy_array(ec->atomics[(int) (*i)[0]], data->temp_features);
      }


      // set example to right namespace only
      ec->indices.erase();
      ec->indices.push_back((int) (*i)[1]);

      // store feature values for right namespace
      copy_array(data->temp_features, ec->atomics[(int) (*i)[1]]);

      for (size_t k = 1; k <= all->rank; k++) {

	// multiply features in right namespace by l^k * x_l
	for (feature* f = ec->atomics[(int) (*i)[1]].begin; f != ec->atomics[(int) (*i)[1]].end; f++)
	  f->x *= data->sub_predictions[2*k-1];

	// update r^k using base learner
	base.learn(ec, k + all->rank);

	// restore right namespace features
	copy_array(ec->atomics[(int) (*i)[1]], data->temp_features);
      }
    }
  }
  // restore namespace indices and unset precomputed prediction
  copy_array(ec->indices, data->indices);

  ec->precomputed_prediction = false;
}

void finish(void* data) {
  mf* o = (mf*) data;
  // restore global pairs
  o->all->pairs = o->pairs;

  // clean up local v_arrays
  o->indices.delete_v();
  o->sub_predictions.delete_v();
}


learner* setup(vw& all, po::variables_map& vm) {
  mf* data = new mf;

  // copy global data locally
  data->all = &all;
  data->rank = all.rank;

  // store global pairs in local data structure and clear global pairs
  // for eventual calls to base learner
  data->pairs = all.pairs;
  all.pairs.clear();

  // initialize weights randomly
  if(!vm.count("initial_regressor"))
    {
      for (size_t j = 0; j < (all.reg.weight_mask + 1) / all.reg.stride; j++)
	all.reg.weight_vector[j*all.reg.stride] = (float) (0.1 * frand48());
    }
  learner* l = new learner(data, learn, all.l, 2*data->rank+1);
  l->set_finish(finish);
  return l;
}
}

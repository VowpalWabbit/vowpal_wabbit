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
  learner base;
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

void inline_predict(mf* data, vw* all, example* &ec) {
  // store namespace indices
  copy_array(data->indices, ec->indices);

  float prediction = 0;
  data->sub_predictions.erase();

  // set label to FLT_MAX to indicate test example (predict only)
  float label = ((label_data*) ec->ld)->label;
  ((label_data*) ec->ld)->label = FLT_MAX;

  // predict from linear terms
  data->base.learn(ec);

  // store linear prediction
  data->sub_predictions.push_back(ec->partial_prediction);
  prediction += ec->partial_prediction;

  // add interaction terms to prediction
  for (vector<string>::iterator i = data->pairs.begin(); i != data->pairs.end(); i++) {
    if (ec->atomics[(int) (*i)[0]].size() > 0 && ec->atomics[(int) (*i)[1]].size() > 0) {
      for (size_t k = 1; k <= all->rank; k++) {
	// set example to left namespace only
	ec->indices.erase();
	ec->indices.push_back((int) (*i)[0]);

	// compute l^k * x_l using base learner
	update_example_indicies(all->audit, ec, data->increment*k);
	data->base.learn(ec);
	float x_dot_l = ec->partial_prediction;
	data->sub_predictions.push_back(ec->partial_prediction);
	update_example_indicies(all->audit, ec, -data->increment*k);

	// set example to right namespace only
	ec->indices.erase();
	ec->indices.push_back((int) (*i)[1]);

	// compute r^k * x_r using base learner
	update_example_indicies(all->audit, ec, data->increment*(k + data->rank));
	data->base.learn(ec);
	float x_dot_r = ec->partial_prediction;
	data->sub_predictions.push_back(ec->partial_prediction);
	update_example_indicies(all->audit, ec, -data->increment*(k + data->rank));

	// accumulate prediction
	prediction += (x_dot_l * x_dot_r);
      }
    }
  }

  // restore namespace indices and label
  copy_array(ec->indices, data->indices);
  ((label_data*) ec->ld)->label = label;

  // finalize prediction
  ec->partial_prediction = prediction;
  ec->final_prediction = GD::finalize_prediction(*(data->all), ec->partial_prediction);

}


void learn(void* d, example* ec) {
  mf* data = (mf*) d;
  vw* all = data->all;

  if (command_example(all, ec)) {
    data->base.learn(ec);
    return;
  }

  // store namespace indices
  copy_array(data->indices, ec->indices);

  // predict with current weights
  inline_predict(data, all, ec);

  // force base learner to use precomputed prediction
  ec->precomputed_prediction = true;

  // update linear weights
  data->base.learn(ec);

  // update interaction terms
  // looping over all pairs of non-empty namespaces
  for (vector<string>::iterator i = data->pairs.begin(); i != data->pairs.end(); i++) {
    if (ec->atomics[(int) (*i)[0]].size() > 0 && ec->atomics[(int) (*i)[1]].size() > 0) {
      for (size_t k = 1; k <= all->rank; k++) {
	// set example to left namespace only
	ec->indices.erase();
	ec->indices.push_back((int) (*i)[0]);

	// store feature values for left namespace
	copy_array(data->temp_features, ec->atomics[(int) (*i)[0]]);

	// multiply features in left namespace by r^k * x_r
	for (feature* f = ec->atomics[(int) (*i)[0]].begin; f != ec->atomics[(int) (*i)[0]].end; f++)
	  f->x *= data->sub_predictions[2*k];

	// update l^k using base learner
	update_example_indicies(all->audit, ec, data->increment*k);
	data->base.learn(ec);
	update_example_indicies(all->audit, ec, -data->increment*k);

	// restore left namespace features
	copy_array(ec->atomics[(int) (*i)[0]], data->temp_features);

	// set example to right namespace only
	ec->indices.erase();
	ec->indices.push_back((int) (*i)[1]);

	// store feature values for right namespace
	copy_array(data->temp_features, ec->atomics[(int) (*i)[1]]);

	// multiply features in right namespace by l^k * x_l
	for (feature* f = ec->atomics[(int) (*i)[1]].begin; f != ec->atomics[(int) (*i)[1]].end; f++)
	  f->x *= data->sub_predictions[2*k-1];

	// update r^k using base learner
	update_example_indicies(all->audit, ec, data->increment*(k + data->rank));
	data->base.learn(ec);
	update_example_indicies(all->audit, ec, -data->increment*(k + data->rank));

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
  o->base.finish();

  // clean up local v_arrays
  o->indices.delete_v();
  o->sub_predictions.delete_v();
  delete o;
}

void drive(vw* all, void* d) {
  example* ec = NULL;

  while (true) {
    if ((ec = VW::get_example(all->p)) != NULL) //blocking operation.
      {
	learn(d, ec);
	return_simple_example(*all, ec);
      } else if (parser_done(all->p))
      return;
    else
      ; //busywait when we have predicted on all examples but not yet trained on all.
  }
}

learner setup(vw& all, po::variables_map& vm) {
  mf* data = new mf;

  // copy global data locally
  data->base = all.l;
  data->all = &all;
  data->rank = all.rank;

  // store global pairs in local data structure and clear global pairs
  // for eventual calls to base learner
  data->pairs = all.pairs;
  all.pairs.clear();

  // set index increment between weights
  data->increment = all.reg.stride * all.weights_per_problem;
  all.weights_per_problem *= data->rank;

  // initialize weights randomly
  if(!vm.count("initial_regressor"))
    {
      for (size_t j = 0; j < (all.reg.weight_mask + 1) / all.reg.stride; j++)
	all.reg.weight_vector[j*all.reg.stride] = (float) (0.1 * frand48());
    }

  learner ret(data, drive,learn, finish, all.l.sl);
  return ret;
}
}

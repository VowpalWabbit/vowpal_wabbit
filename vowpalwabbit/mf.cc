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
  v_array<float> sub_predictions;

  // array for temp storage of indices
  v_array<unsigned char> indices;

  // array for temp storage of features
  v_array<feature> temp_features;

  vw* all;
};

void inline_predict(mf* data, vw* all, example* &ec) {
  copy_array(data->indices, ec->indices);

  float prediction = 0;

  data->sub_predictions.erase();

  // modify label to indicate test example (predict only)
  float label = ((label_data*) ec->ld)->label;
  ((label_data*) ec->ld)->label = FLT_MAX;

  // prediction from linear terms
  data->base.learn(ec);

  // store constant + linear prediction
  data->sub_predictions.push_back(ec->partial_prediction);

  /*
  // prediction from interaction terms
  for (vector<string>::iterator i = data->pairs.begin(); i != data->pairs.end(); i++) {
    if (ec->atomics[(int) (*i)[0]].size() > 0 && ec->atomics[(int) (*i)[1]].size() > 0) {
      for (size_t k = 1; k <= all->rank; k++) {
	// set to left namespace only
	ec->indices.erase();
	ec->indices.push_back((int) (*i)[0]);

	// compute l_k * x_l
	update_example_indicies(all->audit, ec, data->increment*k);
	data->base.learn(ec);
	float x_dot_l = ec->partial_prediction;
	data->sub_predictions.push_back(ec->partial_prediction);
	update_example_indicies(all->audit, ec, -data->increment*k);

	// set to right namespace only
	ec->indices.erase();
	ec->indices.push_back((int) (*i)[1]);

	// compute r_k * x_r
	update_example_indicies(all->audit, ec, data->increment*(k + data->rank));
	data->base.learn(ec);
	float x_dot_r = ec->partial_prediction;
	data->sub_predictions.push_back(ec->partial_prediction);
	update_example_indicies(all->audit, ec, -data->increment*(k + data->rank));

	prediction += (x_dot_l * x_dot_r);
      }
    }
  }
  */
  copy_array(ec->indices, data->indices);

  ((label_data*) ec->ld)->label = label;
  ec->partial_prediction = data->sub_predictions[0] + prediction;
  ec->final_prediction = GD::finalize_prediction(*(data->all), ec->partial_prediction);

}

  float sgn(float x) {
    return 2*(x >= 0) - 1;
  }

void learn_with_output(void* d, example* ec, bool shouldOutput) {
  mf* data = (mf*) d;
  vw* all = data->all;

  if (command_example(all, ec)) {
    data->base.learn(ec);
    return;
  }

  inline_predict(data, all, ec);

  // update linear weights
  ec->precomputed_prediction = true;
  data->base.learn(ec);

  copy_array(data->indices, ec->indices);

  /*
  // update left and right terms
  for (vector<string>::iterator i = data->pairs.begin(); i != data->pairs.end(); i++) {
    if (ec->atomics[(int) (*i)[0]].size() > 0 && ec->atomics[(int) (*i)[1]].size() > 0) {
      for (size_t k = 1; k <= all->rank; k++) {
	// set to left namespace only
	ec->indices.erase();
	ec->indices.push_back((int) (*i)[0]);

	copy_array(data->temp_features, ec->atomics[(int) (*i)[0]]);

	// modify features
	// multiple features in left namespace by r^k * x_r
	for (feature* f = ec->atomics[(int) (*i)[0]].begin; f != ec->atomics[(int) (*i)[0]].end; f++)
	  f->x *= data->sub_predictions[2*k];

	// update l^k
	update_example_indicies(all->audit, ec, data->increment*k);
	data->base.learn(ec);
	update_example_indicies(all->audit, ec, -data->increment*k);

	// restore features
	copy_array(ec->atomics[(int) (*i)[0]], data->temp_features);
	// for (feature* f = ec->atomics[(int) (*i)[0]].begin; f != ec->atomics[(int) (*i)[0]].end; f++)
	//  f->x /= data->sub_predictions[2*k];		   

	// set to right namespace only
	ec->indices.erase();
	ec->indices.push_back((int) (*i)[1]);

	copy_array(data->temp_features, ec->atomics[(int) (*i)[1]]);

	// modify features
	// multiple features in right namespace by l^k * x_l
	for (feature* f = ec->atomics[(int) (*i)[1]].begin; f != ec->atomics[(int) (*i)[1]].end; f++)
	  f->x *= data->sub_predictions[2*k-1];

	// update r^k
	update_example_indicies(all->audit, ec, data->increment*(k + data->rank));
	data->base.learn(ec);
	update_example_indicies(all->audit, ec, -data->increment*(k + data->rank));

	// restore features
	copy_array(ec->atomics[(int) (*i)[1]], data->temp_features);
	// for (feature* f = ec->atomics[(int) (*i)[1]].begin; f != ec->atomics[(int) (*i)[1]].end; f++)
	//  f->x /= data->sub_predictions[2*k-1];

      }
    }
  }
  */
  
  copy_array(ec->indices, data->indices);
  ec->precomputed_prediction = false;

}

void learn(void* d, example* ec) {
	learn_with_output(d, ec, false);
}

void finish(void* data) {
  mf* o = (mf*) data;
  // restore global pairs
  o->all->pairs = o->pairs;
  o->base.finish();

  //cout << "constant " << o->all->reg.weight_vector[(constant * o->all->reg.stride)&o->all->reg.weight_mask] << endl;

  // clean up local arrays
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
  //mf* data = (mf*) calloc(1, sizeof(mf));
  mf* data = new mf;

  data->base = all.l;
  data->all = &all;
  data->rank = all.rank;

  // clear global pairs for eventual calls to base learner
  // store pairs in local data structure
  data->pairs = all.pairs;
  all.pairs.clear();

  // set increment
  data->increment = all.reg.stride * all.weights_per_problem;
  all.weights_per_problem *= data->rank;

  // initialize weights randomly
  if(!vm.count("initial_regressor"))
    {
      for (size_t j = 0; j < (all.reg.weight_mask + 1) / all.reg.stride; j++)
	all.reg.weight_vector[j*all.reg.stride] = (float) (0.1 * frand48());
    }

  //cout << "constant " << data->all->reg.weight_vector[(constant * data->all->reg.stride)&data->all->reg.weight_mask] << endl;

  learner ret(data, drive,learn, finish, all.l.sl);
  return ret;
}
}

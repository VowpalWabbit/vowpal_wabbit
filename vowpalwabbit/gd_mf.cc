/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <fstream>
#include <float.h>
#include <string.h>
#include <stdio.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <netdb.h>
#endif

#include "gd.h"
#include "rand48.h"
#include "reductions.h"
#include "vw_exception.h"
#include "array_parameters.h"


using namespace std;

using namespace LEARNER;

struct gdmf
{ vw* all;//regressor, printing
  v_array<float> scalars;
  uint32_t rank;
  size_t no_win_counter;
  uint64_t early_stop_thres;
};

void mf_print_offset_features(gdmf& d, example& ec, size_t offset)
{
	vw& all = *d.all;
	parameters& weights = all.weights;
	uint64_t mask = weights.mask();
	for (features& fs : ec)
	{
		bool audit = !fs.space_names.empty();
		for (auto& f : fs.values_indices_audit())
		{
			cout << '\t';
			if (audit)
				cout << f.audit().get()->first << '^' << f.audit().get()->second << ':';
			cout << f.index() << "(" << ((f.index() + offset) & mask) << ")" << ':' << f.value();
			cout << ':' << (&weights[f.index()])[offset];
		}
	}
	for (string& i : all.pairs)
		if (ec.feature_space[(unsigned char)i[0]].size() > 0 && ec.feature_space[(unsigned char)i[1]].size() > 0)
		{ /* print out nsk^feature:hash:value:weight:nsk^feature^:hash:value:weight:prod_weights */
			for (size_t k = 1; k <= d.rank; k++)
			{
				for (features::iterator_all& f1 : ec.feature_space[(unsigned char)i[0]].values_indices_audit())
					for (features::iterator_all& f2 : ec.feature_space[(unsigned char)i[1]].values_indices_audit())
					{
						cout << '\t' << f1.audit().get()->first << k << '^' << f1.audit().get()->second << ':' << ((f1.index() + k)&mask)
							<< "(" << ((f1.index() + offset + k) & mask) << ")" << ':' << f1.value();
						cout << ':' << (&weights[f1.index()])[offset + k];

						cout << ':' << f2.audit().get()->first << k << '^' << f2.audit().get()->second << ':' << ((f2.index() + k + d.rank)&mask)
							<< "(" << ((f2.index() + offset + k + d.rank) & mask) << ")" << ':' << f2.value();
								cout << ':' << (&weights[f2.index()])[offset + k + d.rank];

						cout << ':' << (&weights[f1.index()])[offset + k] * (&weights[f2.index()])[offset + k + d.rank];
					}
			}
		}
	if (all.triples.begin() != all.triples.end())
		THROW("cannot use triples in matrix factorization");
	cout << endl;
}

void mf_print_audit_features(gdmf& d, example& ec, size_t offset)
{ print_result(d.all->stdout_fileno,ec.pred.scalar,-1,ec.tag);
  mf_print_offset_features(d, ec, offset);
}

struct pred_offset
{ float p;
  uint64_t offset;
};

void offset_add(pred_offset& res, const float fx, float& fw) { res.p += (&fw)[res.offset] * fx; }

template<class T> float mf_predict(gdmf& d, example& ec, T& weights)
{ vw& all = *d.all;
  label_data& ld = ec.l.simple;
  float prediction = ld.initial;

  for (string& i : d.all->pairs)
  { ec.num_features -= ec.feature_space[(int)i[0]].size() * ec.feature_space[(int)i[1]].size();
    ec.num_features += ec.feature_space[(int)i[0]].size() * d.rank;
    ec.num_features += ec.feature_space[(int)i[1]].size() * d.rank;
  }

  // clear stored predictions
  d.scalars.erase();

  float linear_prediction = 0.;
  // linear terms

  for (features& fs : ec)
	GD::foreach_feature<float, GD::vec_add, T>(weights, fs, linear_prediction);

  // store constant + linear prediction
  // note: constant is now automatically added
  d.scalars.push_back(linear_prediction);

  prediction += linear_prediction;
  // interaction terms
  for (string& i : d.all->pairs)
  { if (ec.feature_space[(int)i[0]].size() > 0 && ec.feature_space[(int)i[1]].size() > 0)
    { for (uint64_t k = 1; k <= d.rank; k++)
      { // x_l * l^k
        // l^k is from index+1 to index+d.rank
        //float x_dot_l = sd_offset_add(weights, ec.atomics[(int)(*i)[0]].begin(), ec.atomics[(int)(*i)[0]].end(), k);
        pred_offset x_dot_l = {0.,k};
        GD::foreach_feature<pred_offset, offset_add, T>(weights, ec.feature_space[(int)i[0]], x_dot_l);
        // x_r * r^k
        // r^k is from index+d.rank+1 to index+2*d.rank
        //float x_dot_r = sd_offset_add(weights, ec.atomics[(int)(*i)[1]].begin(), ec.atomics[(int)(*i)[1]].end(), k+d.rank);
        pred_offset x_dot_r = {0.,k+d.rank};
        GD::foreach_feature<pred_offset,offset_add, T>(weights, ec.feature_space[(int)i[1]], x_dot_r);

        prediction += x_dot_l.p * x_dot_r.p;

        // store prediction from interaction terms
        d.scalars.push_back(x_dot_l.p);
        d.scalars.push_back(x_dot_r.p);
      }
    }
  }

  if (all.triples.begin() != all.triples.end())
    THROW("cannot use triples in matrix factorization");

  // d.scalars has linear, x_dot_l_1, x_dot_r_1, x_dot_l_2, x_dot_r_2, ...

  ec.partial_prediction = prediction;

  all.set_minmax(all.sd, ld.label);

  ec.pred.scalar = GD::finalize_prediction(all.sd, ec.partial_prediction);

  if (ld.label != FLT_MAX)
    ec.loss = all.loss->getLoss(all.sd, ec.pred.scalar, ld.label) * ec.weight;

  if (all.audit)
    mf_print_audit_features(d, ec, 0);

  return ec.pred.scalar;
}

float mf_predict(gdmf& d, example& ec)
{
	vw& all = *d.all;
	if (all.weights.sparse)
		return mf_predict(d, ec, all.weights.sparse_weights);
	else
		return mf_predict(d, ec, all.weights.dense_weights);
}

template<class T>
void sd_offset_update(T& weights, features& fs, uint64_t offset, float update, float regularization)
{ for (size_t i = 0; i < fs.size(); i++)
    (&weights[fs.indicies[i]])[offset] += update * fs.values[i] - regularization * (&weights[fs.indicies[i]])[offset];
}

template<class T>
void mf_train(gdmf& d, example& ec, T& weights)
{ vw& all = *d.all;
  label_data& ld = ec.l.simple;

  // use final prediction to get update size
  // update = eta_t*(y-y_hat) where eta_t = eta/(3*t^p) * importance weight
  float eta_t = all.eta/powf((float) all.sd->t + ec.weight, (float)all.power_t) / 3.f * ec.weight;
  float update = all.loss->getUpdate(ec.pred.scalar, ld.label, eta_t, 1.); //ec.total_sum_feat_sq);

  float regularization = eta_t * all.l2_lambda;

  // linear update
  for (features& fs: ec)
    sd_offset_update<T>(weights, fs, 0, update, regularization);

  // quadratic update
  for (string& i : all.pairs)
  { if (ec.feature_space[(int)i[0]].size() > 0 && ec.feature_space[(int)i[1]].size() > 0)
    {

      // update l^k weights
      for (size_t k = 1; k <= d.rank; k++)
      { // r^k \cdot x_r
        float r_dot_x = d.scalars[2*k];
        // l^k <- l^k + update * (r^k \cdot x_r) * x_l
        sd_offset_update<T>(weights, ec.feature_space[(int)i[0]], k, update*r_dot_x, regularization);
      }
      // update r^k weights
      for (size_t k = 1; k <= d.rank; k++)
      { // l^k \cdot x_l
        float l_dot_x = d.scalars[2*k-1];
        // r^k <- r^k + update * (l^k \cdot x_l) * x_r
        sd_offset_update<T>(weights, ec.feature_space[(int)i[1]], k+d.rank, update*l_dot_x, regularization);
      }

    }
  }
  if (all.triples.begin() != all.triples.end())
    THROW("cannot use triples in matrix factorization");
}

void mf_train(gdmf& d, example& ec)
{
	if (d.all->weights.sparse)
		mf_train(d, ec, d.all->weights.sparse_weights);
	else
		mf_train(d, ec, d.all->weights.dense_weights);
}

template <class T> class set_rand_wrapper 
{
public:
    
    static void func(typename T::iterator& iter, uint32_t& stride)
    {
      uint64_t index = iter.index();
      for (weight_iterator_iterator w = iter.begin(); w != iter.end(stride); ++w, ++index)
        *w = (float)(0.1 * merand48(index));
    }
};

void save_load(gdmf& d, io_buf& model_file, bool read, bool text)
{ vw& all = *d.all;
  uint64_t length = (uint64_t)1 << all.num_bits;
  if(read)
    { initialize_regressor(all);
      if (all.random_weights)
	{
	  uint32_t stride = all.weights.stride();
	  if (all.weights.sparse)
	    all.weights.sparse_weights.set_default<uint32_t, set_rand_wrapper<sparse_parameters> >(stride);
	  else
	    all.weights.dense_weights.set_default<uint32_t, set_rand_wrapper<dense_parameters> >(stride);
	}
    }

  if (model_file.files.size() > 0)
  { uint64_t i = 0;
     size_t brw = 1;
    do
    { brw = 0;
      size_t K = d.rank*2+1;
      stringstream msg;
      msg << i << " ";
      brw += bin_text_read_write_fixed(model_file,(char *)&i, sizeof (i),
                                       "", read, msg, text);
	  if (brw != 0)
	    { weight* w_i= &(all.weights.strided_index(i));
	      for (uint64_t k = 0; k < K; k++)
		{  weight* v = w_i + k;
		  msg << v << " ";
		  brw += bin_text_read_write_fixed(model_file, (char *)v, sizeof(*v),
						   "", read, msg, text);
		}
	    }
      if (text)
        {
          msg << "\n";
          brw += bin_text_read_write_fixed(model_file, nullptr, 0,
                                           "", read, msg,text);
        }

	  if (!read)
	    ++i;
    }
    while ((!read && i < length) || (read && brw >0));
  }
}

void end_pass(gdmf& d)
{ vw* all = d.all;

  all->eta *= all->eta_decay_rate;
  if (all->save_per_pass)
    save_predictor(*all, all->final_regressor_name, all->current_pass);

  all->current_pass++;

  if(!all->holdout_set_off)
  { if(summarize_holdout_set(*all, d.no_win_counter))
      finalize_regressor(*all, all->final_regressor_name);
    if((d.early_stop_thres == d.no_win_counter) &&
        ((all->check_holdout_every_n_passes <= 1) ||
         ((all->current_pass % all->check_holdout_every_n_passes) == 0)))
      set_done(*all);
  }
}

void predict(gdmf& d, base_learner&, example& ec) { mf_predict(d,ec); }

void learn(gdmf& d, base_learner&, example& ec)
{ vw& all = *d.all;

  mf_predict(d, ec);
  if (all.training && ec.l.simple.label != FLT_MAX)
    mf_train(d, ec);
}

void finish(gdmf& d) { d.scalars.delete_v();}

base_learner* gd_mf_setup(vw& all)
{ if (missing_option<uint32_t, true>(all, "rank", "rank for matrix factorization."))
    return nullptr;

  if (all.vm.count("adaptive"))
    THROW("adaptive is not implemented for matrix factorization");
  if (all.vm.count("normalized"))
    THROW("normalized is not implemented for matrix factorization");
  if (all.vm.count("exact_adaptive_norm"))
    THROW("normalized adaptive updates is not implemented for matrix factorization");
  if (all.vm.count("bfgs") || all.vm.count("conjugate_gradient"))
    THROW("bfgs is not implemented for matrix factorization");

  gdmf& data = calloc_or_throw<gdmf>();
  data.all = &all;
  data.rank = all.vm["rank"].as<uint32_t>();
  data.no_win_counter = 0;
  data.early_stop_thres = 3;

  // store linear + 2*rank weights per index, round up to power of two
  float temp = ceilf(logf((float)(data.rank*2+1)) / logf (2.f));
  all.weights.stride_shift((size_t) temp);
  all.random_weights = true;

  if(!all.holdout_set_off)
  { all.sd->holdout_best_loss = FLT_MAX;
    if(all.vm.count("early_terminate"))
      data.early_stop_thres = all.vm["early_terminate"].as< size_t>();
  }

  if(!all.vm.count("learning_rate") && !all.vm.count("l"))
    all.eta = 10; //default learning rate to 10 for non default update rule

  //default initial_t to 1 instead of 0
  if(!all.vm.count("initial_t"))
  { all.sd->t = 1.f;
    all.initial_t = 1.f;
  }
  all.eta *= powf((float)(all.sd->t), all.power_t);

  learner<gdmf>& l = init_learner(&data, learn, UINT64_ONE << all.weights.stride_shift());
  l.set_predict(predict);
  l.set_save_load(save_load);
  l.set_end_pass(end_pass);
  l.set_finish(finish);

  return make_base(l);
}

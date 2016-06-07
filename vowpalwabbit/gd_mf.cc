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

using namespace std;

using namespace LEARNER;

struct gdmf
{ vw* all;//regressor, printing
  uint32_t rank;
  size_t no_win_counter;
  uint64_t early_stop_thres;
};

void mf_print_offset_features(gdmf& d, example& ec, size_t offset)
{ vw& all = *d.all;
  weight* weights = all.reg.weight_vector;
  uint64_t mask = all.reg.weight_mask;
  for (features& fs : ec)
  { bool audit = !fs.space_names.empty();
    for (auto& f : fs.values_indices_audit())
    { cout << '\t';
      if (audit)
        cout << f.audit().get()->first << '^' << f.audit().get()->second << ':';
      cout << f.index() <<"(" << ((f.index() + offset) & mask)  << ")" << ':' << f.value();
      cout << ':' << weights[(f.index() + offset) & mask];
    }
  }
  for (string& i : all.pairs)
    if (ec.feature_space[(unsigned char)i[0]].size() > 0 && ec.feature_space[(unsigned char)i[1]].size() > 0)
    { /* print out nsk^feature:hash:value:weight:nsk^feature^:hash:value:weight:prod_weights */
      for (size_t k = 1; k <= d.rank; k++)
      {
        for (features::iterator_all& f1 : ec.feature_space[(unsigned char)i[0]].values_indices_audit())
          for (features::iterator_all& f2 : ec.feature_space[(unsigned char)i[1]].values_indices_audit())
          { cout << '\t' << f1.audit().get()->first << k << '^' << f1.audit().get()->second << ':' << ((f1.index()+k)&mask)
                 <<"(" << ((f1.index() + offset +k) & mask)  << ")" << ':' << f1.value();
            cout << ':' << weights[(f1.index() + offset + k) & mask];

            cout << ':' << f2.audit().get()->first << k << '^' << f2.audit().get()->second << ':' << ((f2.index() + k + d.rank)&mask)
                 <<"(" << ((f2.index() + offset +k+d.rank) & mask)  << ")" << ':' << f2.value();
            cout << ':' << weights[(f2.index() + offset + k+d.rank) & mask];

            cout << ':' <<  weights[(f1.index() + offset + k) & mask] * weights[(f2.index() + offset + k + d.rank) & mask];
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

float mf_predict(gdmf& d, example& ec)
{ vw& all = *d.all;
  label_data& ld = ec.l.simple;
  float prediction = ld.initial;

  for (string& i : d.all->pairs)
  { ec.num_features -= ec.feature_space[(int)i[0]].size() * ec.feature_space[(int)i[1]].size();
    ec.num_features += ec.feature_space[(int)i[0]].size() * d.rank;
    ec.num_features += ec.feature_space[(int)i[1]].size() * d.rank;
  }

  // clear stored predictions
  ec.topic_predictions.erase();

  float linear_prediction = 0.;
  // linear terms
  for (features& fs : ec)
    GD::foreach_feature<float, GD::vec_add>(all.reg.weight_vector, all.reg.weight_mask, fs, linear_prediction);

  // store constant + linear prediction
  // note: constant is now automatically added
  ec.topic_predictions.push_back(linear_prediction);

  prediction += linear_prediction;

  // interaction terms
  for (string& i : d.all->pairs)
  { if (ec.feature_space[(int)i[0]].size() > 0 && ec.feature_space[(int)i[1]].size() > 0)
    { for (uint64_t k = 1; k <= d.rank; k++)
      { // x_l * l^k
        // l^k is from index+1 to index+d.rank
        //float x_dot_l = sd_offset_add(weights, mask, ec.atomics[(int)(*i)[0]].begin(), ec.atomics[(int)(*i)[0]].end(), k);
        float x_dot_l = 0.;
        GD::foreach_feature<float, GD::vec_add>(all.reg.weight_vector, all.reg.weight_mask, ec.feature_space[(int)i[0]], x_dot_l, k);
        // x_r * r^k
        // r^k is from index+d.rank+1 to index+2*d.rank
        //float x_dot_r = sd_offset_add(weights, mask, ec.atomics[(int)(*i)[1]].begin(), ec.atomics[(int)(*i)[1]].end(), k+d.rank);
        float x_dot_r = 0.;
        GD::foreach_feature<float,GD::vec_add>(all.reg.weight_vector, all.reg.weight_mask, ec.feature_space[(int)i[1]], x_dot_r, k+d.rank);

        prediction += x_dot_l * x_dot_r;

        // store prediction from interaction terms
        ec.topic_predictions.push_back(x_dot_l);
        ec.topic_predictions.push_back(x_dot_r);
      }
    }
  }

  if (all.triples.begin() != all.triples.end())
    THROW("cannot use triples in matrix factorization");

  // ec.topic_predictions has linear, x_dot_l_1, x_dot_r_1, x_dot_l_2, x_dot_r_2, ...

  ec.partial_prediction = prediction;

  all.set_minmax(all.sd, ld.label);

  ec.pred.scalar = GD::finalize_prediction(all.sd, ec.partial_prediction);

  if (ld.label != FLT_MAX)
    ec.loss = all.loss->getLoss(all.sd, ec.pred.scalar, ld.label) * ec.weight;

  if (all.audit)
    mf_print_audit_features(d, ec, 0);

  return ec.pred.scalar;
}


void sd_offset_update(weight* weights, uint64_t mask, features& fs, uint64_t offset, float update, float regularization)
{ for (size_t i = 0; i < fs.size(); i++)
    weights[(fs.indicies[i] + offset) & mask] += update * fs.values[i] - regularization * weights[(fs.indicies[i] + offset) & mask];
}

void mf_train(gdmf& d, example& ec)
{ vw& all = *d.all;
  weight* weights = all.reg.weight_vector;
  uint64_t mask = all.reg.weight_mask;
  label_data& ld = ec.l.simple;

  // use final prediction to get update size
  // update = eta_t*(y-y_hat) where eta_t = eta/(3*t^p) * importance weight
  float eta_t = all.eta/pow(ec.example_t,all.power_t) / 3.f * ec.weight;
  float update = all.loss->getUpdate(ec.pred.scalar, ld.label, eta_t, 1.); //ec.total_sum_feat_sq);

  float regularization = eta_t * all.l2_lambda;

  // linear update
  for (features& fs: ec)
    sd_offset_update(weights, mask, fs, 0, update, regularization);

  // quadratic update
  for (string& i : all.pairs)
  { if (ec.feature_space[(int)i[0]].size() > 0 && ec.feature_space[(int)i[1]].size() > 0)
    {

      // update l^k weights
      for (size_t k = 1; k <= d.rank; k++)
      { // r^k \cdot x_r
        float r_dot_x = ec.topic_predictions[2*k];
        // l^k <- l^k + update * (r^k \cdot x_r) * x_l
        sd_offset_update(weights, mask, ec.feature_space[(int)i[0]], k, update*r_dot_x, regularization);
      }
      // update r^k weights
      for (size_t k = 1; k <= d.rank; k++)
      { // l^k \cdot x_l
        float l_dot_x = ec.topic_predictions[2*k-1];
        // r^k <- r^k + update * (l^k \cdot x_l) * x_r
        sd_offset_update(weights, mask, ec.feature_space[(int)i[1]], k+d.rank, update*l_dot_x, regularization);
      }

    }
  }
  if (all.triples.begin() != all.triples.end())
    THROW("cannot use triples in matrix factorization");
}

void save_load(gdmf& d, io_buf& model_file, bool read, bool text)
{ vw* all = d.all;
  uint64_t length = (uint64_t)1 << all->num_bits;
  uint64_t stride_shift = all->reg.stride_shift;

  if(read)
  { initialize_regressor(*all);
    if(all->random_weights)
      for (size_t j = 0; j < (length << stride_shift); j++)
        all->reg.weight_vector[j] = (float) (0.1 * frand48());
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
        for (uint64_t k = 0; k < K; k++)
        { uint64_t ndx = (i << stride_shift)+k;

          weight* v = &(all->reg.weight_vector[ndx]);
          msg << v << " ";
          brw += bin_text_read_write_fixed(model_file,(char *)v, sizeof (*v),
                                           "", read, msg, text);

        }
      if (text)
        {
          msg << "\n";
          brw += bin_text_read_write_fixed(model_file, nullptr, 0,
                                           "", read, msg,text);
        }

      if (!read)
        i++;
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
  all.reg.stride_shift = (size_t) temp;
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
    all.sd->weighted_unlabeled_examples = 1.f;
    all.initial_t = 1.f;
  }
  all.eta *= powf((float)(all.sd->t), all.power_t);

  learner<gdmf>& l = init_learner(&data, learn, 1 << all.reg.stride_shift);
  l.set_predict(predict);
  l.set_save_load(save_load);
  l.set_end_pass(end_pass);

  return make_base(l);
}

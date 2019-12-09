// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <string>
#include "reductions.h"
#include "rand48.h"
#include "parse_args.h"  // for spoof_hex_encoded_namespaces

using namespace LEARNER;
using namespace VW::config;

struct LRQFAstate
{
  vw* all;
  std::string field_name;
  int k;
  int field_id[256];
  size_t orig_size[256];
};

inline float cheesyrand(uint64_t x)
{
  uint64_t seed = x;

  return merand48(seed);
}

constexpr inline bool example_is_test(example& ec) { return ec.l.simple.label == FLT_MAX; }

template <bool is_learn>
void predict_or_learn(LRQFAstate& lrq, single_learner& base, example& ec)
{
  vw& all = *lrq.all;

  memset(lrq.orig_size, 0, sizeof(lrq.orig_size));
  for (namespace_index i : ec.indices) lrq.orig_size[i] = ec.feature_space[i].size();

  size_t which = ec.example_counter;
  float first_prediction = 0;
  float first_loss = 0;
  unsigned int maxiter = (is_learn && !example_is_test(ec)) ? 2 : 1;
  unsigned int k = lrq.k;
  float sqrtk = (float)std::sqrt(k);

  uint32_t stride_shift = lrq.all->weights.stride_shift();
  uint64_t weight_mask = lrq.all->weights.mask();
  for (unsigned int iter = 0; iter < maxiter; ++iter, ++which)
  {
    // Add left LRQ features, holding right LRQ features fixed
    //     and vice versa

    for (std::string::const_iterator i1 = lrq.field_name.begin(); i1 != lrq.field_name.end(); ++i1)
    {
      for (std::string::const_iterator i2 = i1 + 1; i2 != lrq.field_name.end(); ++i2)
      {
        unsigned char left = (which % 2) ? *i1 : *i2;
        unsigned char right = ((which + 1) % 2) ? *i1 : *i2;
        unsigned int lfd_id = lrq.field_id[left];
        unsigned int rfd_id = lrq.field_id[right];
        for (unsigned int lfn = 0; lfn < lrq.orig_size[left]; ++lfn)
        {
          features& fs = ec.feature_space[left];
          float lfx = fs.values[lfn];
          uint64_t lindex = fs.indicies[lfn];
          for (unsigned int n = 1; n <= k; ++n)
          {
            uint64_t lwindex =
                (lindex + ((uint64_t)(rfd_id * k + n) << stride_shift));  // a feature has k weights in each field
            float* lw = &all.weights[lwindex & weight_mask];
            // perturb away from saddle point at (0, 0)
            if (is_learn && !example_is_test(ec) && *lw == 0)
              *lw = cheesyrand(lwindex) * 0.5f / sqrtk;

            for (unsigned int rfn = 0; rfn < lrq.orig_size[right]; ++rfn)
            {
              features& rfs = ec.feature_space[right];
              //                    feature* rf = ec.atomics[right].begin + rfn;
              // NB: ec.ft_offset added by base learner
              float rfx = rfs.values[rfn];
              uint64_t rindex = rfs.indicies[rfn];
              uint64_t rwindex = (rindex + ((uint64_t)(lfd_id * k + n) << stride_shift));

              rfs.push_back(*lw * lfx * rfx, rwindex);
              if (all.audit || all.hash_inv)
              {
                std::stringstream new_feature_buffer;
                new_feature_buffer << right << '^' << rfs.space_names[rfn].get()->second << '^' << n;
#ifdef _WIN32
                char* new_space = _strdup("lrqfa");
                char* new_feature = _strdup(new_feature_buffer.str().c_str());
#else
                char* new_space = strdup("lrqfa");
                char* new_feature = strdup(new_feature_buffer.str().c_str());
#endif
                rfs.space_names.push_back(audit_strings_ptr(new audit_strings(new_space, new_feature)));
              }
            }
          }
        }
      }
    }

    if (is_learn)
      base.learn(ec);
    else
      base.predict(ec);

    // Restore example
    if (iter == 0)
    {
      first_prediction = ec.pred.scalar;
      first_loss = ec.loss;
    }
    else
    {
      ec.pred.scalar = first_prediction;
      ec.loss = first_loss;
    }

    for (char i : lrq.field_name)
    {
      namespace_index right = i;
      features& rfs = ec.feature_space[right];
      rfs.values.end() = rfs.values.begin() + lrq.orig_size[right];

      if (all.audit || all.hash_inv)
      {
        for (size_t j = lrq.orig_size[right]; j < rfs.space_names.size(); ++j) rfs.space_names[j].~audit_strings_ptr();

        rfs.space_names.end() = rfs.space_names.begin() + lrq.orig_size[right];
      }
    }
  }
}

LEARNER::base_learner* lrqfa_setup(options_i& options, vw& all)
{
  std::string lrqfa;
  option_group_definition new_options("Low Rank Quadratics FA");
  new_options.add(make_option("lrqfa", lrqfa).keep().help("use low rank quadratic features with field aware weights"));
  options.add_and_parse(new_options);

  if (!options.was_supplied("lrqfa"))
    return nullptr;

  auto lrq = scoped_calloc_or_throw<LRQFAstate>();
  lrq->all = &all;

  std::string lrqopt = spoof_hex_encoded_namespaces(lrqfa);
  size_t last_index = lrqopt.find_last_not_of("0123456789");
  new (&lrq->field_name) std::string(lrqopt.substr(0, last_index + 1));  // make sure there is no duplicates
  lrq->k = atoi(lrqopt.substr(last_index + 1).c_str());

  int fd_id = 0;
  for (char i : lrq->field_name) lrq->field_id[(int)i] = fd_id++;

  all.wpp = all.wpp * (uint64_t)(1 + lrq->k);
  learner<LRQFAstate, example>& l = init_learner(lrq, as_singleline(setup_base(options, all)), predict_or_learn<true>,
      predict_or_learn<false>, 1 + lrq->field_name.size() * lrq->k);

  return make_base(l);
}

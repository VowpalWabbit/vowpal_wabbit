#include <string>
#include "reductions.h"
#include "rand48.h"
#include "parse_args.h" // for spoof_hex_encoded_namespaces

using namespace LEARNER;

struct LRQFAstate {
  vw* all;
  string field_name;
  int k;
  int field_id[256];
  size_t orig_size[256];
};

inline float
cheesyrand (uint32_t x)
{
  uint64_t seed = x;

  return merand48 (seed);
}

inline bool
example_is_test (example& ec) {
  return ec.l.simple.label == FLT_MAX;
}

template <bool is_learn>
void predict_or_learn(LRQFAstate& lrq, base_learner& base, example& ec) {
  vw& all = *lrq.all;

  memset(lrq.orig_size, 0, sizeof(lrq.orig_size));
  for (unsigned char* i = ec.indices.begin; i != ec.indices.end; ++i) {
    lrq.orig_size[*i] = ec.atomics[*i].size();
  }

  size_t which = ec.example_counter;
  float first_prediction = 0;
  float first_loss = 0;
  unsigned int maxiter = (is_learn && ! example_is_test (ec)) ? 2 : 1;
  unsigned int k = lrq.k;
  float sqrtk = (float) sqrt(k);
  for (unsigned int iter = 0; iter < maxiter; ++iter, ++which) {
    // Add left LRQ features, holding right LRQ features fixed
    //     and vice versa

    for (string::const_iterator i1 = lrq.field_name.begin(); i1 != lrq.field_name.end(); ++i1) {
      for (string::const_iterator i2 = i1 + 1; i2 != lrq.field_name.end(); ++i2) {
        unsigned char left = which%2 ? *i1 : *i2;
        unsigned char right = (which+1)%2 ? *i1 : *i2;
        unsigned int lfd_id = lrq.field_id[left];
        unsigned int rfd_id = lrq.field_id[right];
        for (unsigned int lfn = 0; lfn < lrq.orig_size[left]; ++lfn) {
          feature* lf = ec.atomics[left].begin + lfn;
          float lfx = lf->x;
          size_t lindex = lf->weight_index;
          for (unsigned int n = 1; n <= k; ++n) {
            uint32_t lwindex = (uint32_t)(lindex + ((rfd_id*k+n) << all.reg.stride_shift)); // a feature has k weights in each field
            float* lw = &all.reg.weight_vector[lwindex & all.reg.weight_mask];

            // perturb away from saddle point at (0, 0)
            if (is_learn && ! example_is_test (ec) && *lw == 0) {
              *lw = cheesyrand(lwindex) * 0.5f / sqrtk;
            }

            for (unsigned int rfn = 0; rfn < lrq.orig_size[right]; ++rfn) {
              feature* rf = ec.atomics[right].begin + rfn;
              audit_data* ra = ec.audit_features[right].begin + rfn;
              // NB: ec.ft_offset added by base learner
              float rfx = rf->x;
              size_t rindex = rf->weight_index;
              uint32_t rwindex = (uint32_t)(rindex + ((lfd_id*k+n) << all.reg.stride_shift));

              feature lrq;
              lrq.x = *lw * lfx * rfx;
              lrq.weight_index = rwindex;
              ec.atomics[right].push_back (lrq);
              if (all.audit || all.hash_inv) {
                std::stringstream new_feature_buffer;
                new_feature_buffer << right << '^'
                                   << ra->feature << '^'
                                   << n;
#ifdef _WIN32
                char* new_space = _strdup("lrqfa");
                char* new_feature = _strdup(new_feature_buffer.str().c_str());
#else
                char* new_space = strdup("lrqfa");
                char* new_feature = strdup(new_feature_buffer.str().c_str());
#endif
                audit_data ad = { new_space, new_feature, lrq.weight_index, lrq.x, true };
                ec.audit_features[right].push_back(ad);
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
    if (iter == 0) {
      first_prediction = ec.pred.scalar;
      first_loss = ec.loss;
    } else {
      ec.pred.scalar = first_prediction;
      ec.loss = first_loss;
    }

    for (string::const_iterator i = lrq.field_name.begin(); i != lrq.field_name.end(); ++i) {
      unsigned char right = *i;

      ec.atomics[right].end = ec.atomics[right].begin + lrq.orig_size[right];

      if (all.audit || all.hash_inv) {
        for (audit_data* a = ec.audit_features[right].begin + lrq.orig_size[right]; a < ec.audit_features[right].end; ++a) {
          free (a->space);
          free (a->feature);
        }

        ec.audit_features[right].end = ec.audit_features[right].begin + lrq.orig_size[right];
      }
    }
  }
}


LEARNER::base_learner* lrqfa_setup(vw& all) {
  if (missing_option<string>(all, "lrqfa", "use low rank quadratic features with field aware weights"))
    return nullptr;

  LRQFAstate& lrq = calloc_or_throw<LRQFAstate>();
  lrq.all = &all;

  string lrqopt = spoof_hex_encoded_namespaces( all.vm["lrqfa"].as<string>() );
  size_t last_index = lrqopt.find_last_not_of("0123456789");
  new(&lrq.field_name) string(lrqopt.substr(0, last_index+1)); // make sure there is no duplicates
  lrq.k = atoi(lrqopt.substr(last_index+1).c_str());

  *all.file_options << " --lrqfa " << lrq.field_name << lrq.k;

  int fd_id = 0;
  for(string::const_iterator i = lrq.field_name.begin(); i != lrq.field_name.end(); ++i) {
    lrq.field_id[(int)*i] = fd_id++;
  }

  all.wpp = all.wpp * (uint32_t)(1 + lrq.k);
  learner<LRQFAstate>& l = init_learner(&lrq, setup_base(all), predict_or_learn<true>, predict_or_learn<false>, 1 + lrq.field_name.size() * lrq.k);

  return make_base(l);
}

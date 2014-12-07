#include <string.h>

#include <float.h>
#include "reductions.h"
#include "rand48.h"

using namespace LEARNER;

namespace LRQ {

  struct LRQstate {
    vw* all;
    bool lrindices[256];
    size_t orig_size[256];
    std::vector<std::string> lrpairs;
    bool dropout;
    uint64_t seed;
    uint64_t initial_seed;
  };
}

namespace {
  bool 
  valid_int (const char* s)
    {
      char* endptr;

      int v = strtoul (s, &endptr, 0);
      (void) v;

      return (*s != '\0' && *endptr == '\0');
    }

  inline bool
  cheesyrbit (uint64_t& seed)
    {
      return merand48 (seed) > 0.5;
    }

  inline float
  cheesyrand (uint32_t x)
    {
      uint64_t seed = x;

      return merand48 (seed);
    }

  inline bool
  example_is_test (example& ec)
  {
    return ec.l.simple.label == FLT_MAX;
  }

  void
  reset_seed (LRQ::LRQstate& lrq)
    {
      if (lrq.all->bfgs)
        lrq.seed = lrq.initial_seed;
    }
}

namespace LRQ {

  template <bool is_learn>
  void predict_or_learn(LRQstate& lrq, learner& base, example& ec)
  {
    vw& all = *lrq.all;

    // Remember original features
        
    for (unsigned char* i = ec.indices.begin; i != ec.indices.end; ++i)
      {
        if (lrq.lrindices[*i])
          lrq.orig_size[*i] = ec.atomics[*i].size ();
      }

    size_t which = ec.example_counter;
    float first_prediction;
    float first_loss;
    unsigned int maxiter = (is_learn && ! example_is_test (ec)) ? 2 : 1;

    bool do_dropout = lrq.dropout && is_learn && ! example_is_test (ec);
    float scale = (! lrq.dropout || do_dropout) ? 1.f : 0.5f;

    for (unsigned int iter = 0; iter < maxiter; ++iter, ++which)
      {
        // Add left LRQ features, holding right LRQ features fixed
        //     and vice versa
        // TODO: what happens with --lrq ab2 --lrq ac2
        //       i.e. namespace occurs multiple times (?)
    
        for (vector<string>::iterator i = lrq.lrpairs.begin ();
             i != lrq.lrpairs.end ();
             ++i)
          {
            unsigned char left = (*i)[which%2];
            unsigned char right = (*i)[(which+1)%2];
            unsigned int k = atoi (i->c_str () + 2);

            for (unsigned int lfn = 0; lfn < lrq.orig_size[left]; ++lfn)
              {
                feature* lf = ec.atomics[left].begin + lfn;
                float lfx = lf->x;
                size_t lindex = lf->weight_index + ec.ft_offset;
    
                for (unsigned int n = 1; n <= k; ++n)
                  {
                    if (! do_dropout || cheesyrbit (lrq.seed))
                      {
                        uint32_t lwindex = (uint32_t)(lindex + (n << all.reg.stride_shift));

                        float* lw = &all.reg.weight_vector[lwindex & all.reg.weight_mask];

                        // perturb away from saddle point at (0, 0)
                        if (is_learn && ! example_is_test (ec) && *lw == 0)
                          *lw = cheesyrand (lwindex);
        
                        for (unsigned int rfn = 0; 
                             rfn < lrq.orig_size[right]; 
                             ++rfn)
                          {
                            feature* rf = ec.atomics[right].begin + rfn;
                            audit_data* ra = ec.audit_features[right].begin + rfn;

                            // NB: ec.ft_offset added by base learner
                            float rfx = rf->x;
                            size_t rindex = rf->weight_index;
                            uint32_t rwindex = (uint32_t)(rindex + (n << all.reg.stride_shift));
        
                            feature lrq; 
                            lrq.x = scale * *lw * lfx * rfx;
                            lrq.weight_index = rwindex; 

                            ec.atomics[right].push_back (lrq);

                            if (iter == 0 && (all.audit || all.hash_inv))
                              {
                                char* new_space = (char*)calloc_or_die(4, sizeof(char));
                                strcpy(new_space, "lrq");
                                size_t n_len = strlen(i->c_str () + 4);
                                size_t len = strlen(ra->feature) + n_len + 2;
                                char* new_feature = (char*)calloc_or_die(len, sizeof(char));
                                new_feature[0] = right;
                                new_feature[1] = '^';
                                strcat(new_feature, ra->feature);
                                strcat(new_feature, "^");
                                sprintf(new_feature+strlen(new_feature), "%d", n);
                                audit_data ad = { new_space, new_feature, lrq.weight_index, lrq.x, true };
                                ec.audit_features[right].push_back (ad);
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

        for (vector<string>::iterator i = lrq.lrpairs.begin ();
             i != lrq.lrpairs.end ();
             ++i)
          {
            unsigned char right = (*i)[(which+1)%2];

            ec.atomics[right].end = 
              ec.atomics[right].begin + lrq.orig_size[right];

            if (all.audit)
              ec.audit_features[right].end = 
                ec.audit_features[right].begin + lrq.orig_size[right];
          }
      }
  }

  learner* setup(vw& all, po::variables_map& vm)
  {//parse and set arguments
    LRQstate* lrq = (LRQstate*) calloc (1, sizeof (LRQstate));
    unsigned int maxk = 0;
    lrq->all = &all;

    size_t random_seed = 0;
    if (vm.count("random_seed")) random_seed = vm["random_seed"].as<size_t> ();

    lrq->initial_seed = lrq->seed = random_seed | 8675309;
	if (vm.count("lrqdropout"))
		lrq->dropout = true;
	else
		lrq->dropout = false;

    all.file_options.append(" --lrqdropout");
    
    lrq->lrpairs = vm["lrq"].as<vector<string> > ();
    
    stringstream ss;
    for (vector<string>::iterator i = lrq->lrpairs.begin (); 
	 i != lrq->lrpairs.end (); 
	 ++i)
      ss << " --lrq " << *i;
    
    all.file_options.append(ss.str());
    

    if (! all.quiet)
      {
        cerr << "creating low rank quadratic features for pairs: ";
        if (lrq->dropout)
          cerr << "(using dropout) ";
      }

    for (vector<string>::iterator i = lrq->lrpairs.begin (); 
         i != lrq->lrpairs.end (); 
         ++i)
      {
        if(!all.quiet){
          if (( i->length() < 3 ) || ! valid_int (i->c_str () + 2)) {
            cerr << endl << "error, low-rank quadratic features must involve two sets and a rank.\n";
            throw exception();
          }
          cerr << *i << " ";
        }
        // TODO: colon-syntax
        
        unsigned int k = atoi (i->c_str () + 2);

        lrq->lrindices[(int) (*i)[0]] = 1;
        lrq->lrindices[(int) (*i)[1]] = 1;

        maxk = max (maxk, k);
      }

    if(!all.quiet)
      cerr<<endl;
        
    all.wpp = all.wpp * (1 + maxk);
    learner* l = new learner(lrq, all.l, 1 + maxk);
    l->set_learn<LRQstate, predict_or_learn<true> >();
    l->set_predict<LRQstate, predict_or_learn<false> >();
    l->set_end_pass<LRQstate,reset_seed>();

    // TODO: leaks memory ?
    return l;
  }
}

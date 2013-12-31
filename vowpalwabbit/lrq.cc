#include "gd.h"
#include "vw.h"
#include "lrq.h"
#include "rand48.h"
#include <float.h>

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
  example_is_test (example* ec)
    {
      return ec->test_only || (((label_data*) ec->ld)->label == FLT_MAX);
    }

  void
  reset_seed (void* d)
    {
      LRQ::LRQstate* lrq = (LRQ::LRQstate*) d;

      if (lrq->all->bfgs)
        lrq->seed = lrq->initial_seed;
    }
}

namespace LRQ {

  void learn(void* d, learner& base, example* ec)
  {
    LRQstate* lrq = (LRQstate*) d;
    vw& all = *lrq->all;

    // Remember original features
        
    for (unsigned char* i = ec->indices.begin; i != ec->indices.end; ++i)
      {
        if (lrq->lrindices[*i])
          lrq->orig_size[*i] = ec->atomics[*i].size ();
      }

    size_t which = ec->example_counter;
    simple_prediction first_prediction;
    float first_loss;
    unsigned int maxiter = (all.training && ! example_is_test (ec)) ? 2 : 1;

    bool do_dropout = lrq->dropout && all.training && ! example_is_test (ec);
    float scale = (! lrq->dropout || do_dropout) ? 1.f : 0.5f;

    for (unsigned int iter = 0; iter < maxiter; ++iter, ++which)
      {
        // Add left LRQ features, holding right LRQ features fixed
        //     and vice versa
        // TODO: what happens with --lrq ab2 --lrq ac2
        //       i.e. namespace occurs multiple times (?)
    
        for (vector<string>::iterator i = lrq->lrpairs.begin ();
             i != lrq->lrpairs.end ();
             ++i)
          {
            unsigned char left = (*i)[which%2];
            unsigned char right = (*i)[(which+1)%2];
            unsigned int k = atoi (i->c_str () + 2);

            for (unsigned int lfn = 0; lfn < lrq->orig_size[left]; ++lfn)
              {
                feature* lf = ec->atomics[left].begin + lfn;
                float lfx = lf->x;
                size_t lindex = lf->weight_index + ec->ft_offset;
    
                for (unsigned int n = 1; n <= k; ++n)
                  {
                    if (! do_dropout || cheesyrbit (lrq->seed))
                      {
                        uint32_t lwindex = (uint32_t)(lindex + n * all.reg.stride);

                        float* lw = &all.reg.weight_vector[lwindex & all.reg.weight_mask];

                        // perturb away from saddle point at (0, 0)
                        if (all.training && ! example_is_test (ec) && *lw == 0)
                          *lw = cheesyrand (lwindex);
        
                        for (unsigned int rfn = 0; 
                             rfn < lrq->orig_size[right]; 
                             ++rfn)
                          {
                            feature* rf = ec->atomics[right].begin + rfn;

                            // NB: ec->ft_offset added by base learner
                            float rfx = rf->x;
                            size_t rindex = rf->weight_index;
                            uint32_t rwindex = (uint32_t)(rindex + n * all.reg.stride);
        
                            feature lrq; 
                            lrq.x = scale * *lw * lfx * rfx;
                            lrq.weight_index = rwindex; 

                            ec->atomics[right].push_back (lrq);

                            if (all.audit)
                              {
                                char name[4] = { 'l', 'r', 'q', '\0' };
                                char subname[4] = { left, '^', right, '\0' };
                                audit_data ad = { name, subname, lrq.weight_index, lrq.x, false };
                                ec->audit_features[right].push_back (ad);
                              }
                          }
                      }
                  }
              }
          }

        base.learn(ec);//Recursive Call

        // Restore example

        if (iter == 0)
          {
            first_prediction = ec->final_prediction;
            first_loss = ec->loss;
          }
        else
          {
            ec->final_prediction = first_prediction;
            ec->loss = first_loss;
          }

        for (vector<string>::iterator i = lrq->lrpairs.begin ();
             i != lrq->lrpairs.end ();
             ++i)
          {
            unsigned char right = (*i)[(which+1)%2];

            ec->atomics[right].end = 
              ec->atomics[right].begin + lrq->orig_size[right];

            if (all.audit)
              ec->audit_features[right].end = 
                ec->audit_features[right].begin + lrq->orig_size[right];
          }
      }
  }

  learner* setup(vw& all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file)
  {//parse and set arguments
    LRQstate* lrq = (LRQstate*) calloc (1, sizeof (LRQstate));
    unsigned int maxk = 0;
    lrq->all = &all;

    size_t random_seed = 0;
    if (vm.count("random_seed")) random_seed = vm["random_seed"].as<size_t> ();
    if (vm_file.count("random_seed")) random_seed = vm_file["random_seed"].as<size_t> ();

    lrq->initial_seed = lrq->seed = random_seed | 8675309;
    lrq->dropout = vm.count("lrqdropout") || vm_file.count("lrqdropout");

    if (lrq->dropout && !vm_file.count("lrqdropout"))
      all.options_from_file.append("--lrqdropout");

    if (!vm_file.count("lrq"))
      {
        lrq->lrpairs = vm["lrq"].as<vector<string> > ();

        // TODO: doesn't work for non-printable stuff
        
        stringstream ss;
        for (vector<string>::iterator i = lrq->lrpairs.begin (); 
             i != lrq->lrpairs.end (); 
             ++i)
          {
            ss << " --lrq " << *i;
          }

        all.options_from_file.append(ss.str());
      }
    else
      lrq->lrpairs = vm_file["lrq"].as<vector<string> > ();

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
    learner* l = new learner(lrq, learn, all.l, 1 + maxk);
    l->set_end_pass (reset_seed);

    // TODO: leaks memory ?
    return l;
  }
}

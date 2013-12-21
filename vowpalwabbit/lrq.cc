#include "gd.h"
#include "vw.h"
#include "lrq.h"
#include <float.h>

namespace {
  bool 
  valid_int (const char* s)
    {
      char* endptr;

      int v = strtoul (s, &endptr, 0);
      (void) v;

      return (*s != '\0' && *endptr == '\0');
    }

  inline float
  cheesyrand (uint32_t x)
    {
      uint32_t v = 1664525 * x + 1013904223;

      return ((float) v) / 2147483648;
    }

  inline bool
  example_is_test (example* ec)
    {
      return ec->test_only || (((label_data*) ec->ld)->label == FLT_MAX);
    }
}

namespace LRQ {

  struct LRQstate {
    vw* all;
    bool lrindices[256];
    size_t orig_size[256];
    std::vector<std::string> lrpairs;
  };

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
                size_t lindex = 
                  quadratic_constant * lf->weight_index + k * ec->ft_offset;
    
                for (unsigned int n = 1; n <= k; ++n)
                  {
                    uint32_t lwindex = lindex + n * all.reg.stride;

                    float* lw = &all.reg.weight_vector[lwindex & all.reg.weight_mask];

                    // perturb away from saddle point at (0, 0)
                    if (all.training && ! example_is_test (ec) && *lw == 0)
                      *lw = cheesyrand (lwindex);
    
                    for (unsigned int rfn = 0; 
                         rfn < lrq->orig_size[right]; 
                         ++rfn)
                      {
                        feature* rf = ec->atomics[right].begin + rfn;

                        size_t rindex = 
                          quadratic_constant * rf->weight_index + k * ec->ft_offset;
                        uint32_t rwindex = rindex + n * all.reg.stride;
    
                        feature lrq; 
                        lrq.x = *lw;
                        lrq.weight_index = rwindex; 

                        ec->atomics[right].push_back (lrq);

                        if (all.audit)
                          {
                            audit_data ad = { "lrq", "blah", lrq.weight_index, lrq.x, false };
                            ec->audit_features[right].push_back (ad);
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
    lrq->all = &all;

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
      cerr << "creating low rank quadratic features for pairs: ";

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
        
        lrq->lrindices[(int) (*i)[0]] = 1;
        lrq->lrindices[(int) (*i)[1]] = 1;
      }

    if(!all.quiet)
      cerr<<endl;
        
    // TODO: leaks memory ?
    return new learner(lrq, learn, all.l);
  }
}

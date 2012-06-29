#include <float.h>
#include <math.h>
#include <stdio.h>

#include "wap.h"
#include "simple_label.h"
#include "cache.h"
#include "csoaa.h"
#include "oaa.h"

using namespace std;

namespace WAP {
  //nonreentrant
  size_t increment=0;

  void mirror_features(vw& all, example* ec, size_t offset1, size_t offset2)
  {
    for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
      {
        size_t original_length = ec->atomics[*i].index();
        //cerr << "original_length = " << original_length << endl;
        for (size_t j = 0; j < original_length; j++)
          {
            feature* f = &ec->atomics[*i][j];
            feature temp = {- f->x, f->weight_index + offset2};
            f->weight_index += offset1;
            push(ec->atomics[*i], temp);
          }
        ec->sum_feat_sq[*i] *= 2;
        //cerr << "final_length = " << ec->atomics[*i].index() << endl;
      }
    if (all.audit)
      {
        for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
          if (ec->audit_features[*i].begin != ec->audit_features[*i].end)
            {
              size_t original_length = ec->audit_features[*i].index();
              for (size_t j = 0; j < original_length; j++)
                {
                  audit_data* f = &ec->audit_features[*i][j];
                  char* new_space = NULL;
                  if (f->space != NULL)
                    {
                      new_space = (char*)calloc(strlen(f->space)+1,sizeof(char));
                      strcpy(new_space, f->space);
                    }
                  char* new_feature = (char*)calloc(strlen(f->feature)+2,sizeof(char));
                  strcpy(new_feature+1, f->feature);
                  *new_feature = '-';
                  audit_data temp = {new_space, new_feature, f->weight_index + offset2, - f->x, true};
                  f->weight_index += offset1;
                  push(ec->audit_features[*i], temp);
                }
              //cerr << "final_length = " << ec->audit_features[*i].index() << endl;
            }
      }
    ec->num_features *= 2;
    ec->total_sum_feat_sq *= 2;
    //cerr << "total_sum_feat_sq = " << ec->total_sum_feat_sq << endl;
  }

  void unmirror_features(vw& all, example* ec, size_t offset1, size_t offset2)
  {
    for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
      {
        ec->atomics[*i].end = ec->atomics[*i].begin+ec->atomics[*i].index()/2;
        feature* end = ec->atomics[*i].end;
        for (feature* f = ec->atomics[*i].begin; f!= end; f++)
          f->weight_index -= offset1;
        ec->sum_feat_sq[*i] /= 2;
      }
    if (all.audit)
      {
        for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
          if (ec->audit_features[*i].begin != ec->audit_features[*i].end)
            {
              for (audit_data *f = ec->audit_features[*i].begin + ec->audit_features[*i].index()/2; f != ec->audit_features[*i].end; f++)
                {
                  if (f->space != NULL)
                    free(f->space);
                  free(f->feature);
                  f->alloced = false;
                }
              ec->audit_features[*i].end = ec->audit_features[*i].begin+ec->audit_features[*i].index()/2;
              for (audit_data *f = ec->audit_features[*i].begin; f != ec->audit_features[*i].end; f++)
                f->weight_index -= offset1;
            }
      }
    ec->num_features /= 2;
    ec->total_sum_feat_sq /= 2;
  }

  struct float_wclass
  {
    float v;
    CSOAA::wclass ci;
  };
  int fi_compare(const void *e1, const void* e2)
  {
    float_wclass* fi1 = (float_wclass*)e1;
    float_wclass* fi2 = (float_wclass*)e2;
    if (fi1->ci.x > fi2->ci.x)
      return 1;
    else if (fi1->ci.x < fi2->ci.x)
      return -1;
    else
      return 0;
  }
  int fi_compare_i(const void *e1, const void* e2)
  {
    float_wclass* fi1 = (float_wclass*)e1;
    float_wclass* fi2 = (float_wclass*)e2;
    if (fi1->ci.weight_index > fi2->ci.weight_index)
      return 1;
    else if (fi1->ci.weight_index < fi2->ci.weight_index)
      return -1;
    else
      return 0;
  }
  v_array<float_wclass> vs;

  void (*base_learner)(void*, example*) = NULL;

  void train(vw& all, example* ec)
  {
    CSOAA::label* ld = (CSOAA::label*)ec->ld;

    CSOAA::wclass* old_end = ld->costs.end;
    CSOAA::wclass* j = ld->costs.begin; 
    for (CSOAA::wclass *cl = ld->costs.begin; cl != ld->costs.end; cl ++)
      if (cl->x != FLT_MAX)
        *j++ = *cl;
    ld->costs.end = j;
  
    float score = FLT_MAX;
    vs.erase();
    for (CSOAA::wclass *cl = ld->costs.begin; cl != ld->costs.end; cl ++)
      {
        float_wclass temp = {0., *cl};
        if (temp.ci.x < score)
          score = temp.ci.x;
        push(vs, temp);
      }
  
    qsort(vs.begin, vs.index(), sizeof(float_wclass), fi_compare);
  
    for (size_t i = 0; i < ld->costs.index(); i++)
      {
        vs[i].ci.x -= score;
        if (i == 0)
          vs[i].v = 0.;
        else
          vs[i].v = vs[i-1].v + (vs[i].ci.x-vs[i-1].ci.x) / (float)i;
      }
  
    qsort(vs.begin, vs.index(), sizeof(float_wclass), fi_compare_i);

    for (size_t i = 0; i < ld->costs.index(); i++)
      for (size_t j = i+1; j < ld->costs.index(); j++)
        {
          label_data simple_temp;
          simple_temp.weight = fabsf(vs[i].v - vs[j].v);
          if (simple_temp.weight > 1e-5)
            {
              simple_temp.initial = 0.;
	    
              if (vs[i].v < vs[j].v)
                simple_temp.label = 1;
              else
                simple_temp.label = -1;
	    
              ec->ld = &simple_temp;
	    
              ec->partial_prediction = 0.;
              uint32_t myi = vs[i].ci.weight_index;
              uint32_t myj = vs[j].ci.weight_index;

              mirror_features(all, ec,(myi-1)*increment, (myj-1)*increment);

              base_learner(&all, ec);
              unmirror_features(all, ec,(myi-1)*increment, (myj-1)*increment);
            }
        }

    ld->costs.end = old_end;
    ec->ld = ld;
  }

  uint32_t test(vw& all, example* ec)
  {
    uint32_t prediction = 1;
    float score = -FLT_MAX;
  
    CSOAA::label* cost_label = (CSOAA::label*)ec->ld; 

    for (size_t i = 0; i < cost_label->costs.index(); i++)
      {
        label_data simple_temp;
        simple_temp.initial = 0.;
        simple_temp.weight = 0.;
        simple_temp.label = FLT_MAX;
        uint32_t myi = cost_label->costs[i].weight_index;
        if (myi!= 1)
          update_example_indicies(all.audit, ec, increment*(myi-1));
        ec->partial_prediction = 0.;
        ec->ld = &simple_temp;
        base_learner(&all, ec);
        if (myi != 1)
          update_example_indicies(all.audit, ec, -increment*(myi-1));
        if (ec->partial_prediction > score)
          {
            score = ec->partial_prediction;
            prediction = myi;
          }
        cost_label->costs[i].partial_prediction = -ec->partial_prediction;
      }
    return prediction;
  }

  void learn(void* a, example* ec)
  {
    vw* all = (vw*)a;
    CSOAA::label* cost_label = (CSOAA::label*)ec->ld;
    
    uint32_t prediction = test(*all, ec);

    ec->ld = cost_label;
    
    if (cost_label->costs.index() > 0)
      train(*all, ec);
    *(OAA::prediction_t*)&(ec->final_prediction) = prediction;
  }

  void drive_wap(void* in)
  {
    vw* all = (vw*)in;
    example* ec = NULL;
    while ( true )
      {
        if ((ec = get_example(all->p)) != NULL)//semiblocking operation.
          {
            base_learner(all, ec);
            CSOAA::output_example(*all, ec);
	    VW::finish_example(*all, ec);
          }
        else if (parser_done(all->p))
          {
            all->finish(all);
            return;
          }
        else 
          ;
      }
  }

  void parse_flags(vw& all, std::vector<std::string>&, po::variables_map& vm, size_t s)
  {
    *(all.p->lp) = CSOAA::cs_label_parser;
    all.sd->k = s;
    all.driver = drive_wap;
    base_learner = all.learn;
    all.learn = learn;
    increment = (all.length()/all.sd->k) * all.stride;
  }
}

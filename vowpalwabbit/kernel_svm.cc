/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <fstream>
#include <sstream>
#include <float.h>
#ifdef _WIN32
#include <WinSock2.h>
#else
#include <netdb.h>
#endif
#include <string.h>
#include <stdio.h>
#include <assert.h>

#if defined(__SSE2__) && !defined(VW_LDA_NO_SSE)
#include <xmmintrin.h>
#endif

#include "parse_example.h"
#include "constant.h"
#include "gd.h"
#include "kernel_svm.h"
#include "cache.h"
#include "accumulate.h"
#include "learner.h"
#include "vw.h"
#include <map>
#include "memory.h"

#define SVM_KER_LIN 0
#define SVM_KER_RBF 1
#define SVM_KER_POLY 2


using namespace std;
using namespace LEARNER;

namespace KSVM
{

  struct svm_params;

  struct svm_example {
    v_array<float> krow;
    flat_example ex;

    ~svm_example();
    void init_svm_example(flat_example *fec); 
    int compute_kernels(svm_params& params);
    int clear_kernels();
  };

  struct svm_model{    
    size_t num_support;
    v_array<svm_example*> support_vec;
    v_array<float> alpha;
    v_array<float> delta;
  };
  
  
  struct svm_params{
    size_t current_pass;
    bool active;
    bool active_pool_greedy;
    bool para_active;
    double active_c;

    size_t pool_size;
    size_t pool_pos;
    size_t subsample; //NOTE: Eliminating subsample to only support 1/pool_size
    size_t reprocess;

    svm_model* model;
    size_t maxcache;
    //size_t curcache;

    svm_example** pool;
    float lambda;

    void* kernel_params;
    size_t kernel_type;

    size_t local_begin, local_end;
    size_t current_t;

    float loss_sum;
    
    vw* all;
  };



  static size_t num_kernel_evals = 0;
  static size_t num_cache_evals = 0;
  
  void svm_example::init_svm_example(flat_example *fec)
  {
    ex = *fec;
    free(fec);
  }
  
  svm_example::~svm_example()
  {
    krow.delete_v();
    // free flatten example contents
    flat_example *fec = (flat_example*)calloc_or_die(1, sizeof(flat_example));
    *fec = ex;
    free_flatten_example(fec); // free contents of flat example and frees fec.
  }


  float
  kernel_function(const flat_example* fec1, const flat_example* fec2, 
		  void* params, size_t kernel_type);

  int
  svm_example::compute_kernels(svm_params& params)
  {
    int alloc = 0;
    svm_model *model = params.model;
    size_t n = model->num_support;
   
    if (krow.size() < n)
      {
	//computing new kernel values and caching them
	//if(params->curcache + n > params->maxcache)
	//trim_cache(params);
	num_kernel_evals += krow.size();
	//cerr<<"Kernels ";
	for (size_t i=krow.size(); i<n; i++)
	  {	    
	    svm_example *sec = model->support_vec[i];
	    float kv = kernel_function(&ex, &(sec->ex), params.kernel_params, params.kernel_type);
	    krow.push_back(kv);
	    alloc += 1;
	    //cerr<<kv<<" ";
	  }
	//cerr<<endl;
      }
    else
      num_cache_evals += n;
    return alloc;
  }

  int 
  svm_example::clear_kernels()
  {
    int rowsize = (int)krow.size();
    krow.end = krow.begin;
    krow.resize(0);
    return -rowsize;
  }

  
  static int 
  make_hot_sv(svm_params& params, size_t svi)
  {
    svm_model *model = params.model;    
    size_t n = model->num_support;
    if (svi >= model->num_support) 
      cerr << "Internal error at " << __FILE__ << ":" << __LINE__ << endl;
    // rotate params fields
    svm_example *svi_e = model->support_vec[svi];
    int alloc = svi_e->compute_kernels(params);
    float svi_alpha = model->alpha[svi];
    float svi_delta = model->delta[svi];
    for (size_t i=svi; i>0; --i)
      {
	model->support_vec[i] = model->support_vec[i-1]; 
	model->alpha[i] = model->alpha[i-1];
	model->delta[i] = model->delta[i-1];
      }
    model->support_vec[0] = svi_e;
    model->alpha[0] = svi_alpha;
    model->delta[0] = svi_delta;
    // rotate cache    
    for (size_t j=0; j<n; j++)
      {
	svm_example *e = model->support_vec[j];
	size_t rowsize = e->krow.size();
	if (svi < rowsize)
	  {
	    float kv = e->krow[svi];
	    for (size_t i=svi; i>0; --i)
	      e->krow[i] = e->krow[i-1];
	    e->krow[0] = kv;
	  }
	else 
	  {
	    float kv = svi_e->krow[j];
	    e->krow.push_back(0);
	    alloc += 1;
	    for (size_t i=e->krow.size()-1; i>0; --i)
	      e->krow[i] = e->krow[i-1];
	    e->krow[0] = kv;
	  }
      }
    return alloc;
  }

  static int 
  trim_cache(svm_params& params)
  {
    int sz = (int)params.maxcache;
    svm_model *model = params.model;
    size_t n = model->num_support;
    int alloc = 0;
    for (size_t i=0; i<n; i++)
      {
	svm_example *e = model->support_vec[i];
	sz -= (int)e->krow.size();
	if (sz < 0)
	  alloc += e->clear_kernels();
      }
    return alloc;
  }


  int save_load_flat_example(io_buf& model_file, bool read, flat_example*& fec) {
    size_t brw = 1;
    if(read) {
      fec = (flat_example*) calloc_or_die(1, sizeof(flat_example));
      brw = bin_read_fixed(model_file, (char*) fec, sizeof(flat_example), "");

      if(brw > 0) {
	if(fec->tag_len > 0) {
	  fec->tag = (char*) calloc_or_die(fec->tag_len, sizeof(char));	
	  brw = bin_read_fixed(model_file, (char*) fec->tag, fec->tag_len*sizeof(char), "");
	  if(!brw) return 2;
	}
	if(fec->feature_map_len > 0) {
	  fec->feature_map = (feature*) calloc_or_die(fec->feature_map_len, sizeof(feature));
	  brw = bin_read_fixed(model_file, (char*) fec->feature_map, fec->feature_map_len*sizeof(feature), ""); 	  if(!brw) return 3;
	}
      }
      else return 1;
    }
    else {
      brw = bin_write_fixed(model_file, (char*) fec, sizeof(flat_example));

      if(brw > 0) {
	if(fec->tag_len > 0) {
	  brw = bin_write_fixed(model_file, (char*) fec->tag, (uint32_t)fec->tag_len*sizeof(char));
	  if(!brw) {
	    cerr<<fec->tag_len<<" "<<fec->tag<<endl;
	    return 2;
	  }
	}
	if(fec->feature_map_len > 0) {
	  brw = bin_write_fixed(model_file, (char*) fec->feature_map, (uint32_t)fec->feature_map_len*sizeof(feature));    	  if(!brw) return 3;
	}
      }
      else return 1;
    }
    return 0;
  }


  void save_load_svm_model(svm_params& params, io_buf& model_file, bool read, bool text) {  
    svm_model* model = params.model;
    //TODO: check about initialization

    //cerr<<"Save load svm "<<read<<" "<<text<<endl;
    if (model_file.files.size() == 0) return;

    bin_text_read_write_fixed(model_file,(char*)&(model->num_support), sizeof(model->num_support), 
			      "", read, "", 0, text);
    //cerr<<"Read num support "<<model->num_support<<endl;
        
    flat_example* fec;
    if(read)
      model->support_vec.resize(model->num_support);

    for(uint32_t i = 0;i < model->num_support;i++) {
      if(read) {
	save_load_flat_example(model_file, read, fec);
	svm_example* tmp= (svm_example*)calloc_or_die(1,sizeof(svm_example));
	tmp->init_svm_example(fec);
	model->support_vec.push_back(tmp);
      }
      else {
	fec = &(model->support_vec[i]->ex);
	save_load_flat_example(model_file, read, fec);
      }
    }
    //cerr<<endl;
    
    //cerr<<"Read model"<<endl;
    
    if(read)
      model->alpha.resize(model->num_support);
    bin_text_read_write_fixed(model_file, (char*)model->alpha.begin, (uint32_t)model->num_support*sizeof(float),
			      "", read, "", 0, text);
    if(read)
      model->delta.resize(model->num_support);
    bin_text_read_write_fixed(model_file, (char*)model->delta.begin, (uint32_t)model->num_support*sizeof(float),
			      "", read, "", 0, text);        

    // cerr<<"In save_load\n";
    // for(int i = 0;i < model->num_support;i++)
    //   cerr<<model->alpha[i]<<" ";
    // cerr<<endl;
  }

  void save_load(svm_params& params, io_buf& model_file, bool read, bool text) {  
    if(text) {
      cerr<<"Not supporting readable model for kernel svm currently\n";
      return;
    }

    save_load_svm_model(params, model_file, read, text);
    
  }
  
  float linear_kernel(const flat_example* fec1, const flat_example* fec2) {

    float dotprod = 0;
    
    feature* ec2f = fec2->feature_map;
    uint32_t ec2pos = ec2f->weight_index;          
    uint32_t idx1 = 0, idx2 = 0;
    
    //cerr<<"Intersection ";
    int numint = 0;
    for (feature* f = fec1->feature_map; idx1 < fec1->feature_map_len && idx2 < fec2->feature_map_len ; f++, idx1++) {      
      uint32_t ec1pos = f->weight_index;      
      //cerr<<ec1pos<<" "<<ec2pos<<" "<<idx1<<" "<<idx2<<" "<<f->x<<" "<<ec2f->x<<endl;
      if(ec1pos < ec2pos) continue;

      while(ec1pos > ec2pos && idx2 < fec2->feature_map_len) {
	ec2f++;
	idx2++;
	if(idx2 < fec2->feature_map_len)
	  ec2pos = ec2f->weight_index;
      }      

      if(ec1pos == ec2pos) {	
	//cerr<<ec1pos<<" "<<ec2pos<<" "<<idx1<<" "<<idx2<<" "<<f->x<<" "<<ec2f->x<<endl;
	numint++;
	dotprod += f->x*ec2f->x;
	//cerr<<f->x<<" "<<ec2f->x<<" "<<dotprod<<" ";
	ec2f++;
	idx2++;
	//cerr<<idx2<<" ";
	if(idx2 < fec2->feature_map_len)
	  ec2pos = ec2f->weight_index;
      }
    }
    //cerr<<endl;
    //cerr<<"numint = "<<numint<<" dotprod = "<<dotprod<<endl;
    return dotprod;
  }

  float poly_kernel(const flat_example* fec1, const flat_example* fec2, int power) {
    float dotprod = linear_kernel(fec1, fec2);    
    //cerr<<"Bandwidth = "<<bandwidth<<endl;
    //cout<<pow(1 + dotprod, power)<<endl;
    return pow(1 + dotprod, power);
  }

  float rbf_kernel(const flat_example* fec1, const flat_example* fec2, float bandwidth) {
    float dotprod = linear_kernel(fec1, fec2);    
    //cerr<<"Bandwidth = "<<bandwidth<<endl;
    return expf(-(fec1->total_sum_feat_sq + fec2->total_sum_feat_sq - 2*dotprod)*bandwidth);
  }

  float kernel_function(const flat_example* fec1, const flat_example* fec2, void* params, size_t kernel_type) {
    switch(kernel_type) {
    case SVM_KER_RBF:
      return rbf_kernel(fec1, fec2, *((float*)params));
    case SVM_KER_POLY:
      return poly_kernel(fec1, fec2, *((int*)params));
    case SVM_KER_LIN:
      return linear_kernel(fec1, fec2);
    }
    return 0;
  }

  float dense_dot(float* v1, v_array<float> v2, size_t n) {
    float dot_prod = 0.;
    for(size_t i = 0;i < n;i++)
      dot_prod += v1[i]*v2[i];
    return dot_prod;
  }


  void predict (svm_params& params, svm_example** ec_arr, float* scores, size_t n) { 
    svm_model* model = params.model;
    for(size_t i = 0;i < n; i++) {
      ec_arr[i]->compute_kernels(params);
      scores[i] = dense_dot(ec_arr[i]->krow.begin, model->alpha, model->num_support)/params.lambda;
    }
  }
  
  void predict(svm_params& params, learner &base, example& ec) {
    flat_example* fec = flatten_sort_example(*(params.all),&ec);    
    if(fec) {
      svm_example* sec = (svm_example*)calloc_or_die(1, sizeof(svm_example)); 
      sec->init_svm_example(fec);
      float score;
      predict(params, &sec, &score, 1);
      ec.pred.scalar = score;
    }
  }

      
  size_t suboptimality(svm_model* model, double* subopt) {

    size_t max_pos = 0;
    //cerr<<"Subopt ";
    double max_val = 0;
    for(size_t i = 0;i < model->num_support;i++) {
      label_data& ld = model->support_vec[i]->ex.l.simple;
      double tmp = model->alpha[i]*ld.label;                  
      
      if((tmp < ld.weight && model->delta[i] < 0) || (tmp > 0 && model->delta[i] > 0)) 
	subopt[i] = fabs(model->delta[i]);
      else
	subopt[i] = 0;

	if(subopt[i] > max_val) {
	  max_val = subopt[i];
	  max_pos = i;
	}
	//cerr<<subopt[i]<<" ";
      }    
    //cerr<<endl;
    return max_pos;
  }  

  int remove(svm_params& params, size_t svi) {
    svm_model* model = params.model;
    if (svi >= model->num_support) 
      cerr << "Internal error at " << __FILE__ << ":" << __LINE__ << endl;
    // shift params fields
    svm_example* svi_e = model->support_vec[svi];
    for (size_t i=svi; i<model->num_support-1; ++i)
      {
	model->support_vec[i] = model->support_vec[i+1];
	model->alpha[i] = model->alpha[i+1];
	model->delta[i] = model->delta[i+1];
      }
    svi_e->~svm_example();
    free(svi_e);
    model->support_vec.pop();
    model->alpha.pop();
    model->delta.pop();
    model->num_support--;
    // shift cache
    int alloc = 0;
    for (size_t j=0; j<model->num_support; j++)
      {
	svm_example *e = model->support_vec[j];
	size_t rowsize = e->krow.size();
	if (svi < rowsize)
	  {
	    for (size_t i=svi; i<rowsize-1; i++)
	      e->krow[i] = e->krow[i+1];
	    e->krow.pop();
	    alloc -= 1;
	  }
      }
    return alloc;
  }

  int add(svm_params& params, svm_example* fec) {
    svm_model* model = params.model;
    model->num_support++;
    model->support_vec.push_back(fec);
    model->alpha.push_back(0.);
    model->delta.push_back(0.);
    //cout<<"After adding "<<model->num_support<<endl;
    return (int)(model->support_vec.size()-1);
  }

  bool update(svm_params& params, size_t pos) {

    //cerr<<"Update\n";
    svm_model* model = params.model;
    bool overshoot = false;
    //cerr<<"Updating model "<<pos<<" "<<model->num_support<<" ";
    //cerr<<model->support_vec[pos]->example_counter<<endl;
    svm_example* fec = model->support_vec[pos];
    label_data& ld = fec->ex.l.simple;
    fec->compute_kernels(params);
    float *inprods = fec->krow.begin;
    float alphaKi = dense_dot(inprods, model->alpha, model->num_support);
    model->delta[pos] = alphaKi*ld.label/params.lambda - 1;
    float alpha_old = model->alpha[pos];
    alphaKi -= model->alpha[pos]*inprods[pos];
    model->alpha[pos] = 0.;
    
    float proj = alphaKi*ld.label;
    float ai = (params.lambda - proj)/inprods[pos];
    //cerr<<model->num_support<<" "<<pos<<" "<<proj<<" "<<alphaKi<<" "<<alpha_old<<" "<<ld->label<<" "<<model->delta[pos]<<" ";

    if(ai > ld.weight)				
      ai = ld.weight;
    else if(ai < 0)
      ai = 0;

    ai *= ld.label;
    float diff = ai - alpha_old;

    if(fabs(diff) > 1.0e-06) 
      overshoot = true;
    
    if(fabs(diff) > 1.) {
      //cerr<<"Here\n";
      diff = (float) (diff > 0) - (diff < 0);
      ai = alpha_old + diff;
    }
    
    for(size_t i = 0;i < model->num_support; i++) {
      label_data& ldi = model->support_vec[i]->ex.l.simple;
      model->delta[i] += diff*inprods[i]*ldi.label/params.lambda;
    }
    
    if(fabs(ai) <= 1.0e-10)
      remove(params, pos);
    else 
      model->alpha[pos] = ai;

    return overshoot;
  }

  void copy_char(char& c1, const char& c2) {
    if (c2 != '\0')
      c1 = c2;
  }

  void add_size_t(size_t& t1, const size_t& t2) {
    t1 += t2;
  }

  void add_double(double& t1, const double& t2) {
    t1 += t2;
  }

  void sync_queries(vw& all, svm_params& params, bool* train_pool) {
    io_buf* b = new io_buf();
    
    char* queries;
    flat_example* fec;

    for(size_t i = 0;i < params.pool_pos;i++) {
      if(!train_pool[i])
	continue;
      
      fec = &(params.pool[i]->ex);
      save_load_flat_example(*b, false, fec);
      delete params.pool[i];
      
    }

    size_t* sizes = (size_t*) calloc_or_die(all.total, sizeof(size_t));
    sizes[all.node] = b->space.end - b->space.begin;
    //cerr<<"Sizes = "<<sizes[all.node]<<" ";
    all_reduce<size_t, add_size_t>(sizes, all.total, all.span_server, all.unique_id, all.total, all.node, all.socks);

    size_t prev_sum = 0, total_sum = 0;
    for(size_t i = 0;i < all.total;i++) {
      if(i <= (all.node - 1))
	prev_sum += sizes[i];
      total_sum += sizes[i];
    }
    
    //cerr<<total_sum<<" "<<prev_sum<<endl;
    if(total_sum > 0) {
      queries = (char*) calloc_or_die(total_sum, sizeof(char));
      memcpy(queries + prev_sum, b->space.begin, b->space.end - b->space.begin);
      b->space.delete_v();
      all_reduce<char, copy_char>(queries, total_sum, all.span_server, all.unique_id, all.total, all.node, all.socks);

      b->space.begin = queries;
      b->space.end = b->space.begin;
      b->endloaded = &queries[total_sum*sizeof(char)];

      size_t num_read = 0;
      params.pool_pos = 0;
      
      for(size_t i = 0;i < params.pool_size; i++) {	
	if(!save_load_flat_example(*b, true, fec)) {
	  params.pool[i] = (svm_example*)calloc_or_die(1,sizeof(svm_example));
	  params.pool[i]->init_svm_example(fec);
	  train_pool[i] = true;
	  params.pool_pos++;
	  // for(int j = 0;j < fec->feature_map_len;j++)
	  //   cerr<<fec->feature_map[j].weight_index<<":"<<fec->feature_map[j].x<<" ";
	  // cerr<<endl;
	  // params.pool[i]->in_use = true;
	  // params.current_t += ((label_data*) params.pool[i]->ld)->weight;
	  // params.pool[i]->example_t = params.current_t;
	}
	else
	  break;
	
	num_read += b->space.end - b->space.begin;
	if(num_read == prev_sum)
	  params.local_begin = i+1;
	if(num_read == prev_sum + sizes[all.node])
	  params.local_end = i;	
      }
    }
    if(fec)
      free(fec);
    free(sizes);
    delete b;

  }


  void train(svm_params& params) {
    
    //cerr<<"In train "<<params.all->training<<endl;
    
    bool* train_pool = (bool*)calloc_or_die(params.pool_size, sizeof(bool));
    for(size_t i = 0;i < params.pool_size;i++)
      train_pool[i] = false;
    
    float* scores = (float*)calloc_or_die(params.pool_pos, sizeof(float));
    predict(params, params.pool, scores, params.pool_pos);
    //cout<<scores[0]<<endl;
    
      
    if(params.active) {           
      if(params.active_pool_greedy) { 
	multimap<double, size_t> scoremap;
	for(size_t i = 0;i < params.pool_pos; i++)
	  scoremap.insert(pair<const double, const size_t>(fabs(scores[i]),i));

	multimap<double, size_t>::iterator iter = scoremap.begin();
	//cerr<<params.pool_size<<" "<<"Scoremap: ";
	//for(;iter != scoremap.end();iter++)
	//cerr<<iter->first<<" "<<iter->second<<" "<<((label_data*)params.pool[iter->second]->ld)->label<<"\t";
	//cerr<<endl;
	iter = scoremap.begin();
	
	for(size_t train_size = 1;iter != scoremap.end() && train_size <= params.subsample;train_size++) {
	  //cerr<<train_size<<" "<<iter->second<<" "<<iter->first<<endl;
	  train_pool[iter->second] = 1;
	  iter++;	  
	}
      }
      else {

	for(size_t i = 0;i < params.pool_pos;i++) {
	  float queryp = 2.0f/(1.0f + expf((float)(params.active_c*fabs(scores[i]))*pow(params.pool[i]->ex.example_counter,0.5f)));
	  if(rand() < queryp) {
	    svm_example* fec = params.pool[i];
	    fec->ex.l.simple.weight *= 1/queryp;
	    train_pool[i] = 1;
	  }
	}
      }
      //free(scores);
    }

    
    if(params.para_active) {
      for(size_t i = 0;i < params.pool_pos;i++)
	if(!train_pool[i])
	  delete params.pool[i];
      sync_queries(*(params.all), params, train_pool);
    }

    if(params.all->training) {
      
      svm_model* model = params.model;
      
      for(size_t i = 0;i < params.pool_pos;i++) {
	//cerr<<"process: "<<i<<" "<<train_pool[i]<<endl;;
	int model_pos = -1;
	if(params.active) {
	  if(train_pool[i]) {
	    //cerr<<"i = "<<i<<"train_pool[i] = "<<train_pool[i]<<" "<<params.pool[i]->example_counter<<endl;
	    model_pos = add(params, params.pool[i]);
	  }
	}
	else
	  model_pos = add(params, params.pool[i]);	
	
	//cerr<<"Added: "<<model_pos<<" "<<model->support_vec[model_pos]->example_counter<<endl;
	//cout<<"After adding in train "<<model->num_support<<endl;
	
	if(model_pos >= 0) {
	  bool overshoot = update(params, model_pos);
	  //cerr<<model_pos<<":alpha = "<<model->alpha[model_pos]<<endl;

	  double* subopt = (double*)calloc_or_die(model->num_support,sizeof(double));
	  for(size_t j = 0;j < params.reprocess;j++) {
	    if(model->num_support == 0) break;
	    //cerr<<"reprocess: ";
	    int randi = 1;//rand()%2;
	    if(randi) {
	      size_t max_pos = suboptimality(model, subopt);
	      if(subopt[max_pos] > 0) {
		if(!overshoot && max_pos == (size_t)model_pos && max_pos > 0 && j == 0) 
		  cerr<<"Shouldn't reprocess right after process!!!\n";
		//cerr<<max_pos<<" "<<subopt[max_pos]<<endl;
		// cerr<<params.model->support_vec[0]->example_counter<<endl;
		if(max_pos*model->num_support <= params.maxcache)
		  make_hot_sv(params, max_pos);
		update(params, max_pos);
	      }
	    }
	    else {
	      size_t rand_pos = rand()%model->num_support;
	      update(params, rand_pos);
	    }
	  }	  
	  //cerr<<endl;
	  // cerr<<params.model->support_vec[0]->example_counter<<endl;
	  free(subopt);
	}
      }

    }
    else
      for(size_t i = 0;i < params.pool_pos;i++)
	delete params.pool[i];
	
    // cerr<<params.model->support_vec[0]->example_counter<<endl;
    // for(int i = 0;i < params.pool_size;i++)
    //   cerr<<scores[i]<<" ";
    // cerr<<endl;
    free(scores);
    //cerr<<params.model->support_vec[0]->example_counter<<endl;
    free(train_pool);
    //cerr<<params.model->support_vec[0]->example_counter<<endl;
  }

  void learn(svm_params& params, learner& base, example& ec) {
    flat_example* fec = flatten_sort_example(*(params.all),&ec);
    // for(int i = 0;i < fec->feature_map_len;i++)
    //   cout<<i<<":"<<fec->feature_map[i].x<<" "<<fec->feature_map[i].weight_index<<" ";
    // cout<<endl;
    if(fec) {
      svm_example* sec = (svm_example*)calloc_or_die(1, sizeof(svm_example));
      sec->init_svm_example(fec);
      float score = 0;
      predict(params, &sec, &score, 1);
      ec.pred.scalar = score;
      ec.loss = max(0.f, 1.f - score*ec.l.simple.label);
      params.loss_sum += ec.loss;
      if(params.all->training && ec.example_counter % 100 == 0)
	trim_cache(params);
      if(params.all->training && ec.example_counter % 1000 == 0 && ec.example_counter >= 2) {
	cerr<<"Number of support vectors = "<<params.model->num_support<<endl;
	cerr<<"Number of kernel evaluations = "<<num_kernel_evals<<" "<<"Number of cache queries = "<<num_cache_evals<<" loss sum = "<<params.loss_sum<<" "<<params.model->alpha[params.model->num_support-1]<<" "<<params.model->alpha[params.model->num_support-2]<<endl;
      }
      params.pool[params.pool_pos] = sec;
      params.pool_pos++;
      
      if(params.pool_pos == params.pool_size) {
	train(params); 
	params.pool_pos = 0;
      }
    }
  }

  void free_svm_model(svm_model* model)
  {
    for(size_t i = 0;i < model->num_support; i++) {
      model->support_vec[i]->~svm_example();
      free(model->support_vec[i]);
      model->support_vec[i] = 0;
    }

    model->support_vec.delete_v();
    model->alpha.delete_v();
    model->delta.delete_v();
    free(model);
  }

  void finish(svm_params& params) {
    free(params.pool);

    cerr<<"Num support = "<<params.model->num_support<<endl;
    cerr<<"Number of kernel evaluations = "<<num_kernel_evals<<" "<<"Number of cache queries = "<<num_cache_evals<<endl;
    cerr<<"Total loss = "<<params.loss_sum<<endl;

    free_svm_model(params.model);
    cerr<<"Done freeing model\n";
    if(params.kernel_params) free(params.kernel_params);
    cerr<<"Done freeing kernel params\n";
    cerr<<"Done with finish \n";
  }


  LEARNER::learner* setup(vw &all, po::variables_map& vm) {
    po::options_description desc("KSVM options");
    desc.add_options()
      ("reprocess", po::value<size_t>(), "number of reprocess steps for LASVM")
      ("active", "do active learning")
      ("active_c", po::value<double>(), "parameter for query prob")
      ("pool_greedy", "use greedy selection on mini pools")      
      ("para_active", "do parallel active learning")
      ("pool_size", po::value<size_t>(), "size of pools for active learning")
      ("subsample", po::value<size_t>(), "number of items to subsample from the pool")
      ("kernel", po::value<string>(), "type of kernel (rbf or linear (default))")
      ("bandwidth", po::value<float>(), "bandwidth of rbf kernel")
      ("degree", po::value<int>(), "degree of poly kernel")
      ("lambda", po::value<double>(), "saving regularization for test time");
    vm = add_options(all, desc);

    string loss_function = "hinge";
    float loss_parameter = 0.0;
    delete all.loss;
    all.loss = getLossFunction(&all, loss_function, (float)loss_parameter);

    svm_params* params = (svm_params*) calloc_or_die(1,sizeof(svm_params));
    params->model = (svm_model*) calloc_or_die(1,sizeof(svm_model));
    params->model->num_support = 0;
    //params->curcache = 0;
    params->maxcache = 1024*1024*1024;
    params->loss_sum = 0.;
    params->all = &all;
    
    if(vm.count("reprocess"))
      params->reprocess = vm["reprocess"].as<std::size_t>();
    else 
      params->reprocess = 1;

    if(vm.count("active"))
      params->active = true;
    if(params->active) {
      if(vm.count("active_c"))
	params->active_c = vm["active_c"].as<double>();
      else
	params->active_c = 1.;
      if(vm.count("pool_greedy"))
	params->active_pool_greedy = 1;
      /*if(vm.count("para_active"))
	params->para_active = 1;*/
    }
    
    if(vm.count("pool_size")) 
      params->pool_size = vm["pool_size"].as<std::size_t>();
    else
      params->pool_size = 1;
    
    params->pool = (svm_example**)calloc_or_die(params->pool_size, sizeof(svm_example*));
    params->pool_pos = 0;
    
    if(vm.count("subsample"))
	params->subsample = vm["subsample"].as<std::size_t>();
      else if(params->para_active)
	params->subsample = (size_t)ceil(params->pool_size / all.total);
      else
	params->subsample = 1;
    
    params->lambda = all.l2_lambda;

    std::stringstream ss1, ss2;
    ss1 <<" --lambda "<< params->lambda;
    all.file_options.append(ss1.str());
      
    cerr<<"Lambda = "<<params->lambda<<endl;

    std::string kernel_type;

    if(vm.count("kernel")) 
      kernel_type = vm["kernel"].as<std::string>();
    else
      kernel_type = string("linear");
    
    ss2 <<" --kernel "<< kernel_type;
    all.file_options.append(ss2.str());

    cerr<<"Kernel = "<<kernel_type<<endl;

    if(kernel_type.compare("rbf") == 0) {
      params->kernel_type = SVM_KER_RBF;
      float bandwidth = 1.;
      if(vm.count("bandwidth")) {
		std::stringstream ss;
		bandwidth = vm["bandwidth"].as<float>();	
		ss<<" --bandwidth "<<bandwidth;
		all.file_options.append(ss.str());
      }
      cerr<<"bandwidth = "<<bandwidth<<endl;
      params->kernel_params = calloc_or_die(1,sizeof(double*));
      *((float*)params->kernel_params) = bandwidth;
    }
    else if(kernel_type.compare("poly") == 0) {
      params->kernel_type = SVM_KER_POLY;
      int degree = 2;
      if(vm.count("degree")) {
	  std::stringstream ss;
	  degree = vm["degree"].as<int>();	
	  ss<<" --degree "<<degree;
	  all.file_options.append(ss.str());
	}
      cerr<<"degree = "<<degree<<endl;
      params->kernel_params = calloc_or_die(1,sizeof(int*));
      *((int*)params->kernel_params) = degree;
    }      
    else
      params->kernel_type = SVM_KER_LIN;            
	
    params->all->reg.weight_mask = (uint32_t)LONG_MAX;
    params->all->reg.stride_shift = 0;
    
    learner* l = new learner(params, 1); 
    l->set_learn<svm_params, learn>();
    l->set_predict<svm_params, predict>();
    l->set_save_load<svm_params, save_load>();
    l->set_finish<svm_params, finish>();
    return l;
  }    
}

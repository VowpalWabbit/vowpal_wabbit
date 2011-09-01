/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license

The algorithm here is generally based on Nocedal 1980, Liu and Nocedal 1989.
Implementation by Miro Dudik.
 */
#include <fstream>
#include <float.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sys/timeb.h>
#include "parse_example.h"
#include "constant.h"
#include "sparse_dense.h"
#include "bfgs.h"
#include "cache.h"
#include "multisource.h"
#include "simple_label.h"
#include "delay_ring.h"
#include "accumulate.h"

using namespace std;

#define BFGS_EXTRA 4
#define BFGS_XT 0
#define BFGS_GT 1
#define BFGS_QR 2

#define BFGS_W_XT 0
#define BFGS_W_GT 1
#define BFGS_W_DIR 2
#define BFGS_W_COND 3

/********************************************************************/
/* mem & w definition ***********************************************/
/********************************************************************/ 
// mem[2*i] = s_t
// mem[2*i+1] = y_t
//
// w[0] = weight
// w[1] = accumulated first derivative
// w[2] = step direction
// w[3] = preconditioner
  
namespace BFGS 

{

struct timeb t_start, t_end;
double net_comm_time = 0.0;

void quad_grad_update(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask, float g)
{
  size_t halfhash = quadratic_constant * page_feature.weight_index;
  float update = g * page_feature.x;
  for (feature* ele = offer_features.begin; ele != offer_features.end; ele++)
    {
      weight* w=&weights[(halfhash + ele->weight_index) & mask];
      w[1] += update * ele->x;
    }
}

void quad_precond_update(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask, float g)
{
  size_t halfhash = quadratic_constant * page_feature.weight_index;
  float update = g * page_feature.x * page_feature.x;
  for (feature* ele = offer_features.begin; ele != offer_features.end; ele++)
    {
      weight* w=&weights[(halfhash + ele->weight_index) & mask];
      w[3] += update * ele->x * ele->x;
    }
}

// w[0] = weight
// w[1] = accumulated first derivative
// w[2] = step direction
// w[3] = preconditioner

float predict_and_gradient(regressor& reg, example* &ec)
{
  float raw_prediction = inline_predict(reg,ec,0);
  float fp = finalize_prediction(raw_prediction);
  
  label_data* ld = (label_data*)ec->ld;

  float loss_grad = reg.loss->first_derivative(fp,ld->label)*ld->weight;
  
  size_t thread_mask = global.thread_mask;
  weight* weights = reg.weight_vectors[0];
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
    {
      feature *f = ec->subsets[*i][0];
      for (; f != ec->subsets[*i][1]; f++)
	{
	  weight* w = &weights[f->weight_index & thread_mask];
	  w[1] += loss_grad * f->x;
	}
    }
  for (vector<string>::iterator i = global.pairs.begin(); i != global.pairs.end();i++) 
    {
      if (ec->subsets[(int)(*i)[0]].index() > 0)
	{
	  v_array<feature> temp = ec->atomics[(int)(*i)[0]];
	  temp.begin = ec->subsets[(int)(*i)[0]][0];
	  temp.end = ec->subsets[(int)(*i)[0]][1];
	  for (; temp.begin != temp.end; temp.begin++)
	    quad_grad_update(weights, *temp.begin, ec->atomics[(int)(*i)[1]], thread_mask, loss_grad);
	} 
    }
  return fp;
}

void update_preconditioner(regressor& reg, example* &ec)
{
  label_data* ld = (label_data*)ec->ld;
  float curvature = reg.loss->second_derivative(ec->final_prediction,ld->label) * ld->weight;
  
  size_t thread_mask = global.thread_mask;
  weight* weights = reg.weight_vectors[0];
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++)
    {
      feature *f = ec->subsets[*i][0];
      for (; f != ec->subsets[*i][1]; f++)
        {
          weight* w = &weights[f->weight_index & thread_mask];
          w[3] += f->x * f->x * curvature;
        }
    }
  for (vector<string>::iterator i = global.pairs.begin(); i != global.pairs.end();i++)
    {
      if (ec->subsets[(int)(*i)[0]].index() > 0)
        {
          v_array<feature> temp = ec->atomics[(int)(*i)[0]];
          temp.begin = ec->subsets[(int)(*i)[0]][0];
          temp.end = ec->subsets[(int)(*i)[0]][1];
          for (; temp.begin != temp.end; temp.begin++)
            quad_precond_update(weights, *temp.begin, ec->atomics[(int)(*i)[1]], thread_mask, curvature);
        }
    }
}  


float dot_with_direction(regressor& reg, example* &ec)
{
  float ret = 0;
  weight* weights = reg.weight_vectors[0];
  size_t thread_mask = global.thread_mask;
  weights +=2;//direction vector stored two advanced
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
    {
      feature *f = ec->subsets[*i][0];
      for (; f != ec->subsets[*i][1]; f++)
	ret += weights[f->weight_index & thread_mask] * f->x;
    }
  for (vector<string>::iterator i = global.pairs.begin(); i != global.pairs.end();i++) 
    {
      if (ec->subsets[(int)(*i)[0]].index() > 0)
	{
	  v_array<feature> temp = ec->atomics[(int)(*i)[0]];
	  temp.begin = ec->subsets[(int)(*i)[0]][0];
	  temp.end = ec->subsets[(int)(*i)[0]][1];
	  for (; temp.begin != temp.end; temp.begin++)
	    ret += one_pf_quad_predict(weights, *temp.begin, ec->atomics[(int)(*i)[1]], thread_mask);
	} 
    }
  return ret;
}

void zero_derivative(regressor& reg)
{//set derivative to 0.
  uint32_t length = 1 << global.num_bits;
  size_t stride = global.stride;
  weight* weights = reg.weight_vectors[0];
  for(uint32_t i = 0; i < length; i++)
    weights[stride*i+1] = 0;
}

void zero_preconditioner(regressor& reg)
{//set derivative to 0.
  uint32_t length = 1 << global.num_bits;
  size_t stride = global.stride;
  weight* weights = reg.weight_vectors[0];
  for(uint32_t i = 0; i < length; i++)
    weights[stride*i+3] = 0;
}

double regularizer_direction_magnitude(regressor& reg, float regularizer)
{//compute direction magnitude
  double ret = 0.;
  
  if (regularizer == 0.)
    return ret;

  uint32_t length = 1 << global.num_bits;
  size_t stride = global.stride;
  weight* weights = reg.weight_vectors[0];
  if (reg.regularizers == NULL)
    for(uint32_t i = 0; i < length; i++)
      ret += regularizer*weights[stride*i+2]*weights[stride*i+2];
  else
    for(uint32_t i = 0; i < length; i++) 
      ret += reg.regularizers[0][2*i]*weights[stride*i+2]*weights[stride*i+2];

  return ret;
}

double direction_magnitude(regressor& reg)
{//compute direction magnitude
  double ret = 0.;
  uint32_t length = 1 << global.num_bits;
  size_t stride = global.stride;
  weight* weights = reg.weight_vectors[0];
  for(uint32_t i = 0; i < length; i++)
    ret += weights[stride*i+2]*weights[stride*i+2];
  
  return ret;
}

double old_gamma;

void bfgs_iter_start(regressor&reg, float* mem, int& lastj, double importance_weight_sum)
{
  int m = global.m;
  int mem_stride = 2*m+BFGS_EXTRA;
  
  uint32_t length = 1 << global.num_bits;
  size_t stride = global.stride;
  weight* w = reg.weight_vectors[0];

  double g1_Hg1 = 0.;
  double g1_g1 = 0.;
  
  for(uint32_t i = 0; i < length; i++, mem+=mem_stride, w+=stride) {
    mem[2*m+BFGS_XT] = w[BFGS_W_XT];
    mem[2*m+BFGS_GT] = w[BFGS_W_GT];
    g1_Hg1 += w[BFGS_W_GT] * w[BFGS_W_GT] * w[BFGS_W_COND];
    g1_g1 += w[BFGS_W_GT] * w[BFGS_W_GT];
    w[BFGS_W_DIR] = -w[BFGS_W_COND]*w[BFGS_W_GT];
    w[BFGS_W_GT] = 0;
  }
  lastj = 0;
  old_gamma = 1.;
  if (!global.quiet)
    fprintf(stderr, "%-10e\t%-10e\t%-10s\t%-10s\t%-10s\t",
	    g1_g1/(importance_weight_sum*importance_weight_sum),
	    g1_Hg1/importance_weight_sum, "", "", "");
}

void bfgs_iter_middle(regressor&reg, float* mem, double* rho, double* alpha, int& lastj)
{
  int m = global.m;
  int mem_stride = 2*m+BFGS_EXTRA;
  
  uint32_t length = 1 << global.num_bits;
  size_t stride = global.stride;
  weight* w = reg.weight_vectors[0];
  
  float* mem0 = mem;
  float* w0 = w;

  // implement conjugate gradient
  if (m==0) {
    double g_Hy = 0.;
    double g_Hg = 0.;
    double y_s = 0.;
    double y_Hy = 0.;
    double y = 0.;
  
    for(uint32_t i = 0; i < length; i++, mem+=mem_stride, w+=stride) {
      y = w[BFGS_W_GT]-mem[BFGS_GT];
      g_Hy += w[BFGS_W_GT] * w[BFGS_W_COND] * y;
      g_Hg += mem[BFGS_GT] * w[BFGS_W_COND] * mem[BFGS_GT];
      y_s += y * (w[BFGS_W_XT]-mem[BFGS_XT]);
      y_Hy += y * w[BFGS_W_COND] * y;
    }

    double beta = g_Hy/g_Hg;
    double gamma = (global.hessian_on) ? 1.0 : y_s/y_Hy;

    if (beta<0. || isnan(beta))
      beta = 0.;
    if (y_s <= 0. || y_Hy <= 0.) {
      cout << "your curvature is not positive, something wrong.  Try adding regularization" << endl;
      exit(1);
    }
      
    mem = mem0;
    w = w0;
    for(uint32_t i = 0; i < length; i++, mem+=mem_stride, w+=stride) {
      mem[BFGS_XT] = w[BFGS_W_XT];
      mem[BFGS_GT] = w[BFGS_W_GT];

      w[BFGS_W_DIR] *= gamma*beta/old_gamma;
      w[BFGS_W_DIR] -= gamma*w[BFGS_W_COND]*w[BFGS_W_GT];
      w[BFGS_W_GT] = 0;
    }
    old_gamma = gamma;
    if (!global.quiet)
      fprintf(stderr, "%f\t", beta);
    return;
  }
  else {
    if (!global.quiet)
      fprintf(stderr, "%-10s\t","");
  }

  double y_s = 0.;
  double y_Hy = 0.;
  double s_q = 0.;
  
  for(uint32_t i = 0; i < length; i++, mem+=mem_stride, w+=stride) {
    mem[0] = w[BFGS_W_XT] - mem[2*m+BFGS_XT];
    mem[1] = w[BFGS_W_GT] - mem[2*m+BFGS_GT];
    mem[2*m+BFGS_QR] = w[BFGS_GT];
    y_s += mem[1]*mem[0];
    y_Hy += mem[1]*mem[1]*w[BFGS_W_COND];
    s_q += mem[0]*w[BFGS_W_GT];  
  }
  
  if (y_s <= 0. || y_Hy <= 0.) {
    cout << "your curvature is not positive, something wrong.  Try adding regularization" << endl;
    exit(1);
  }

  rho[0] = 1/y_s;
  
  double gamma = y_s/y_Hy;

  for (int j=0; j<lastj; j++) {
    alpha[j] = rho[j] * s_q;
    s_q = 0.;
    mem = mem0;
    for(uint32_t i = 0; i < length; i++, mem+=mem_stride) {
      mem[2*m+BFGS_QR] -= alpha[j]*mem[2*j+1];
      s_q += mem[2*j+2]*mem[2*m+BFGS_QR];
    }
  }

  alpha[lastj] = rho[lastj] * s_q;
  double y_r = 0.;  
  mem = mem0;
  w = w0;
  for(uint32_t i = 0; i < length; i++, mem+=mem_stride, w+=stride) {
    mem[2*m+BFGS_QR] -= alpha[lastj]*mem[2*lastj+1];
    mem[2*m+BFGS_QR] *= gamma*w[BFGS_W_COND];
    y_r += mem[2*lastj+1]*mem[2*m+BFGS_QR];
  }

  double coef_j;
    
  for (int j=lastj; j>0; j--) {
    coef_j = alpha[j] - rho[j] * y_r;
    y_r = 0.;
    mem = mem0;
    for(uint32_t i = 0; i < length; i++, mem+=mem_stride) {
      mem[2*m+BFGS_QR] += coef_j*mem[2*j];
      y_r += mem[2*j-1]*mem[2*m+BFGS_QR];
    }
  }

  coef_j = alpha[0] - rho[0] * y_r;
  mem = mem0;
  w = w0;
  for(uint32_t i = 0; i < length; i++, mem+=mem_stride, w+=stride) {
    w[BFGS_W_DIR] = -mem[2*m+BFGS_QR]-coef_j*mem[0];
  }
  
  /*********************
   ** shift 
   ********************/

  mem = mem0;
  w = w0;
  lastj = (lastj<m-1) ? lastj+1 : m-1;
  for(uint32_t i = 0; i < length; i++, mem+=mem_stride, w+=stride) {
    for (int j=lastj; j>0; j--) {
      mem[2*j]   = mem[2*j-2];
      mem[2*j+1] = mem[2*j-1];
    }
    mem[2*m+BFGS_XT] = w[BFGS_W_XT];
    mem[2*m+BFGS_GT] = w[BFGS_W_GT];
    w[BFGS_W_GT] = 0;
  }
  for (int j=lastj; j>0; j--)
    rho[j] = rho[j-1];
}


double wolfe_eval(regressor& reg, float* mem, double loss_sum, double previous_loss_sum, double step, double importance_weight_sum) {
  int m = global.m;
  int mem_stride = 2*m+BFGS_EXTRA;
  
  uint32_t length = 1 << global.num_bits;
  size_t stride = global.stride;
  weight* w = reg.weight_vectors[0];
  
  double g0_d = 0.;
  double g1_d = 0.;
  double g1_Hg1 = 0.;
  double g1_g1 = 0.;
  
  for(uint32_t i = 0; i < length; i++, mem+=mem_stride, w+=stride) {
    g0_d += mem[2*m+BFGS_GT] * w[BFGS_W_DIR];
    g1_d += w[BFGS_W_GT] * w[BFGS_W_DIR];
    g1_Hg1 += w[BFGS_W_GT] * w[BFGS_W_GT] * w[BFGS_W_COND];
    g1_g1 += w[BFGS_W_GT] * w[BFGS_W_GT];
  }
  
  double wolfe1 = (loss_sum-previous_loss_sum)/(step*g0_d);
  double wolfe2 = g1_d/g0_d;
  double new_step_simple = 0.5*step;
  double new_step_cross  = 0.5*(loss_sum-previous_loss_sum-g1_d*step)/(g0_d-g1_d);

  bool violated = false;
  if (new_step_cross<0. || new_step_cross>1. || isnan(new_step_cross)) {
    violated = true;
    new_step_cross = new_step_simple;
  }

  
  if (!global.quiet)
    fprintf(stderr, "%-10e\t%-10e\t%s%-10f\t%-10f\t", g1_g1/(importance_weight_sum*importance_weight_sum), g1_Hg1/importance_weight_sum, violated ? "*" : " ", wolfe1, wolfe2);
  return new_step_cross;
}


double add_regularization(regressor& reg,float regularization)
{//compute the derivative difference
  double ret = 0.;
  uint32_t length = 1 << global.num_bits;
  size_t stride = global.stride;
  weight* weights = reg.weight_vectors[0];
  if (reg.regularizers == NULL)
    {
      for(uint32_t i = 0; i < length; i++) {
	weights[stride*i+1] += regularization*weights[stride*i];
	ret += 0.5*regularization*weights[stride*i]*weights[stride*i];
      }
    }
  else
    {
      for(uint32_t i = 0; i < length; i++) {
	weight delta_weight = weights[stride*i] - reg.regularizers[0][2*i+1];
	weights[stride*i+1] += reg.regularizers[0][2*i]*delta_weight;
	ret += 0.5*reg.regularizers[0][2*i]*delta_weight*delta_weight;
      }
    }
  return ret;
}

void finalize_preconditioner(regressor& reg,float regularization)
{
  uint32_t length = 1 << global.num_bits;
  size_t stride = global.stride;
  weight* weights = reg.weight_vectors[0];

  if (reg.regularizers == NULL)
    for(uint32_t i = 0; i < length; i++) {
      weights[stride*i+3] += regularization;
      if (weights[stride*i+3] > 0)
	weights[stride*i+3] = 1. / weights[stride*i+3];
    }
  else
    for(uint32_t i = 0; i < length; i++) {
      weights[stride*i+3] += reg.regularizers[0][2*i];
      if (weights[stride*i+3] > 0)
	weights[stride*i+3] = 1. / weights[stride*i+3];
    }
}

void preconditioner_to_regularizer(regressor& reg, float regularization)
{
  uint32_t length = 1 << global.num_bits;
  size_t stride = global.stride;
  weight* weights = reg.weight_vectors[0];
  if (reg.regularizers == NULL)
    {
      size_t num_threads = global.num_threads();
      reg.regularizers = (weight **)malloc(num_threads * sizeof(weight*));
      for (size_t i = 0; i < num_threads; i++)
	{
	  if (reg.regularizers != NULL)
	    reg.regularizers[i] = (weight *)calloc(2*length/num_threads, sizeof(weight));
	  
	  if ((reg.regularizers != NULL && reg.regularizers[i] == NULL))
	    {
	      cerr << global.program_name << ": Failed to allocate weight array: try decreasing -b <bits>" << endl;
	      exit (1);
	    }
	}
      for(uint32_t i = 0; i < length; i++) 
	reg.regularizers[0][2*i] = weights[stride*i+3] + regularization;
    }
  else
    for(uint32_t i = 0; i < length; i++) 
      reg.regularizers[0][2*i] = weights[stride*i+3] + reg.regularizers[0][2*i];
  for(uint32_t i = 0; i < length; i++) 
    reg.regularizers[0][2*i+1] = weights[stride*i];
}

void zero_state(regressor& reg)
{
  uint32_t length = 1 << global.num_bits;
  size_t stride = global.stride;
  weight* weights = reg.weight_vectors[0];
  for(uint32_t i = 0; i < length; i++) 
    {
      weights[stride*i+1] = 0;
      weights[stride*i+2] = 0;
      weights[stride*i+3] = 0;
    }
}

  double derivative_in_direction(regressor& reg, float* mem)
  {
    int m = global.m;
  int mem_stride = 2*m+BFGS_EXTRA;
  
  double ret = 0.;
  uint32_t length = 1 << global.num_bits;
  size_t stride = global.stride;
  weight* w = reg.weight_vectors[0];
  
  for(uint32_t i = 0; i < length; i++, w+=stride, mem+=mem_stride)
    ret += mem[2*m+BFGS_GT]*w[BFGS_W_DIR];
  return ret;
}
  
  void update_weight(string& reg_name, regressor& reg, float step_size, size_t current_pass)
  {
    uint32_t length = 1 << global.num_bits;
    size_t stride = global.stride;
    weight* w = reg.weight_vectors[0];
    
    for(uint32_t i = 0; i < length; i++, w+=stride)
      w[BFGS_W_XT] += step_size * w[BFGS_W_DIR];
    save_predictor(reg_name, current_pass);
  }

  void update_weight_mem(string& reg_name, regressor& reg, float* mem, float step_size, size_t current_pass)
{
  int m = global.m;
  int mem_stride = 2*m+BFGS_EXTRA;
  
  uint32_t length = 1 << global.num_bits;
  size_t stride = global.stride;
  weight* w = reg.weight_vectors[0];

  for(uint32_t i = 0; i < length; i++, w+=stride, mem+=mem_stride)
    w[BFGS_W_XT] = mem[2*m+BFGS_XT] + step_size * w[BFGS_W_DIR];
  save_predictor(reg_name, current_pass);
}

void setup_bfgs(gd_thread_params& t)
{
  regressor reg = t.reg;
  size_t thread_num = 0;
  example* ec = NULL;

  v_array<float> predictions;
  size_t example_number=0;
  double curvature=0.;

  bool gradient_pass=true;
  double loss_sum = 0;
  float step_size = 0.;
  double importance_weight_sum = 0.;
 
  size_t current_pass = 0;
  double previous_loss_sum = 0;

  int m = global.m;
  float* mem = (float*) malloc(sizeof(float)*global.length()*(2*m+BFGS_EXTRA));
  double* rho = (double*) malloc(sizeof(double)*m);
  double* alpha = (double*) malloc(sizeof(double)*m);
  int lastj = 0;

  if (!global.quiet) 
    {
      fprintf(stderr, "m = %d\nAllocated %luM for weights and mem\n", m, (long unsigned int)global.length()*(sizeof(float)*(2*m+BFGS_EXTRA)+sizeof(weight)*global.stride) >> 20);
    }

  struct timeb t_start_global, t_end_global;
  double net_time = 0.0;
  ftime(&t_start_global);
  
  if (!global.quiet)
    {
      const char * header_fmt = "%2s %-10s\t%-10s\t%-10s\t %-10s\t%-10s\t%-10s\t%-10s\t%-10s\t%-10s\t%-10s\n";
      fprintf(stderr, header_fmt,
	      "##", "avg. loss", "der. mag.", "d. m. cond.", "wolfe1", "wolfe2", "mix fraction", "curvature", "dir. magnitude", "step size", "time");
      cerr.precision(5);
    }

  bool output_regularizer = false;
  if (reg.regularizers != NULL)
      global.regularization = 1; // To make sure we are adding the regularization
  output_regularizer =  (global.per_feature_regularizer_output != "" || global.per_feature_regularizer_text != "");
  
  while ( true )
    {
      if ((ec = get_example(thread_num)) != NULL)//semiblocking operation.
	{
	  assert(ec->in_use);
 
 /********************************************************************/
  /* FINISHED A PASS OVER EXAMPLES: DISTINGUISH CASES *****************/
  /********************************************************************/ 
	  if (ec->pass != current_pass) {

  /********************************************************************/
  /* A) FIRST PASS FINISHED: INITIALIZE FIRST LINE SEARCH *************/
  /********************************************************************/ 
	      if (current_pass == 0) {
		if(global.master_location != "")
		  {
		    accumulate(global.master_location, reg, 3); //Accumulate preconditioner
		    importance_weight_sum = accumulate_scalar(global.master_location, importance_weight_sum);
		  }
		finalize_preconditioner(reg,global.regularization);
		if(global.master_location != "") {
		  loss_sum = accumulate_scalar(global.master_location, loss_sum);  //Accumulate loss_sums
		  accumulate(global.master_location, reg, 1); //Accumulate gradients from all nodes
		}
		if (global.regularization > 0.)
		  loss_sum += add_regularization(reg,global.regularization);
		if (!global.quiet)
		  fprintf(stderr, "%2lu %-f\t", (long unsigned int)current_pass+1, loss_sum / importance_weight_sum);
		
		previous_loss_sum = loss_sum;
		loss_sum = 0.;
		example_number = 0;
		curvature = 0;
		bfgs_iter_start(reg, mem, lastj, importance_weight_sum);		     		     
		gradient_pass = false;//now start computing curvature
	      }

  /********************************************************************/
  /* B) NOT FIRST PASS, GRADIENT CALCULATED ***************************/
  /********************************************************************/ 
	      else if (gradient_pass) // We just finished computing all gradients
		{
		  if(global.master_location != "") {
		    loss_sum = accumulate_scalar(global.master_location, loss_sum);  //Accumulate loss_sums
		    accumulate(global.master_location, reg, 1); //Accumulate gradients from all nodes
		  }
		  if (global.regularization > 0.)
		    loss_sum += add_regularization(reg,global.regularization);
		  if (!global.quiet)
		    fprintf(stderr, "%2lu %-f\t", (long unsigned int)current_pass+1, loss_sum / importance_weight_sum);

		  double new_step = wolfe_eval(reg, mem, loss_sum, previous_loss_sum, step_size, importance_weight_sum);
  /********************************************************************/
  /* B1) LINE SEARCH FAILED *******************************************/
  /********************************************************************/ 
		  if (current_pass > 0 && loss_sum > previous_loss_sum)
		    {// we stepped too far last time, step back
		      ftime(&t_end_global);
		      net_time = (int) (1000.0 * (t_end_global.time - t_start_global.time) + (t_end_global.millitm - t_start_global.millitm)); 
		      if (!global.quiet)
			fprintf(stderr, "%-10s\t%-10s\t(revise x %.1f)\t%-10e\t%-.3f\n",
				"","",new_step/step_size,
				new_step,
				net_time/1000.);
			predictions.erase();
			update_weight_mem(*(t.final_regressor_name), reg,mem,new_step, current_pass);
			step_size = new_step;
			zero_derivative(reg);
			loss_sum = 0.;
		    }

  /********************************************************************/
  /* B2) LINE SEARCH SUCCESSFUL                      ******************/
  /*     DETERMINE NEXT SEARCH DIRECTION             ******************/
  /********************************************************************/ 
		  else {
		      previous_loss_sum = loss_sum;
		      loss_sum = 0.;
		      example_number = 0;
		      curvature = 0;

		      bfgs_iter_middle(reg, mem, rho, alpha, lastj);

		      if (global.hessian_on) {
			gradient_pass = false;//now start computing curvature
		      }
		      else {
			float d_mag = direction_magnitude(reg);
			step_size = 1.0;
			ftime(&t_end_global);
			net_time = (int) (1000.0 * (t_end_global.time - t_start_global.time) + (t_end_global.millitm - t_start_global.millitm)); 
			if (!global.quiet)
			  fprintf(stderr, "%-10s\t%-10e\t%-10e\t%-10.3f\n", "", d_mag, step_size, (net_time/1000.));
			predictions.erase();
			update_weight(*(t.final_regressor_name), reg, step_size, current_pass);		     		      
		      }
		    }
		}

  /********************************************************************/
  /* C) NOT FIRST PASS, CURVATURE CALCULATED **************************/
  /********************************************************************/ 
	      else // just finished all second gradients
		{
		  if(global.master_location != "") {
		    curvature = accumulate_scalar(global.master_location, curvature);  //Accumulate curvatures
		  }
		  if (global.regularization > 0.)
		    curvature += regularizer_direction_magnitude(reg,global.regularization);
		  float dd = derivative_in_direction(reg, mem);
		  if (curvature == 0. && dd != 0.)
		    {
		      cout << "your curvature is 0, something wrong.  Try adding regularization" << endl;
		      exit(1);
		    }
		  step_size = - dd/curvature;
		  float d_mag = direction_magnitude(reg);

		  predictions.erase();
		  update_weight(*(t.final_regressor_name),reg,step_size, current_pass);
		  ftime(&t_end_global);
		  net_time = (int) (1000.0 * (t_end_global.time - t_start_global.time) + (t_end_global.millitm - t_start_global.millitm)); 
		  if (!global.quiet)
		    fprintf(stderr, "%-e\t%-e\t%-e\t%-.3f\n", curvature / importance_weight_sum, d_mag, step_size,(net_time/1000.));
		  gradient_pass = true;
		}//now start computing derivatives.
	      
	      current_pass++;
	      
  /********************************************************************/
  /* IN THE LAST PASS CALCULATE THE DIAGONAL OF THE HESSIAN ***********/
  /********************************************************************/ 
	      if (output_regularizer && current_pass == global.numpasses - 1)
		zero_preconditioner(reg);
	  }
  /********************************************************************/
  /* PROCESS AN EXAMPLE: DISTINGUISH CASES ****************************/
  /********************************************************************/ 

  /********************************************************************/
  /* I) GRADIENT CALCULATION ******************************************/
  /********************************************************************/ 
	  if (gradient_pass)
	    {
	      ec->final_prediction = predict_and_gradient(reg,ec);//w[0] & w[1]
	      if (current_pass == 0)
		{
		  label_data* ld = (label_data*)ec->ld;
		  importance_weight_sum += ld->weight;
		  update_preconditioner(reg,ec);//w[3]
		}
	      label_data* ld = (label_data*)ec->ld;
	      ec->loss = reg.loss->getLoss(ec->final_prediction, ld->label) * ld->weight;
	      loss_sum += ec->loss;
	      push(predictions,ec->final_prediction);
	    }
  /********************************************************************/
  /* II) CURVATURE CALCULATION ****************************************/
  /********************************************************************/ 
	  else //computing curvature
	    {
	      float d_dot_x = dot_with_direction(reg,ec);//w[2]
	      label_data* ld = (label_data*)ec->ld;
	      ec->final_prediction = predictions[example_number];
	      ec->loss = reg.loss->getLoss(ec->final_prediction, ld->label) * ld->weight;	      
	      float sd = reg.loss->second_derivative(predictions[example_number++],ld->label);
	      curvature += d_dot_x*d_dot_x*sd*ld->weight;
	    }
	  if (output_regularizer && current_pass == global.numpasses -1)
	    {
	      update_preconditioner(reg,ec);//w[3]
	    }
	  finish_example(ec);
	}

  /********************************************************************/
  /* PROCESS THE FINAL EXAMPLE ****************************************/
  /********************************************************************/ 
     else if (thread_done(thread_num))
	{
	  if (example_number == predictions.index() && gradient_pass)//do one last update
	    {
		  if(global.master_location != "") {
		    loss_sum = accumulate_scalar(global.master_location, loss_sum);  //Accumulate loss_sums
		    accumulate(global.master_location, reg, 1); //Accumulate gradients from all nodes
		  }
		  if (global.regularization > 0.)
		    loss_sum += add_regularization(reg,global.regularization);
		  if (!global.quiet)
		    fprintf(stderr, "%2lu %-f\t", (long unsigned int)current_pass+1, loss_sum / importance_weight_sum);

		  wolfe_eval(reg, mem, loss_sum, previous_loss_sum, step_size, importance_weight_sum);
		  if (current_pass > 0 && loss_sum > previous_loss_sum)
		    {// current weight does not decrease loss
		      if (!global.quiet)
			fprintf(stderr, "\t\t\t\t\t(revise)\t%-e", 0.0);
		      predictions.erase();
		      update_weight_mem(*(t.final_regressor_name),reg,mem,0.0, current_pass);
		    }
	    }
	  if (!global.quiet)
	    fprintf(stderr, "\n");
	  break;
	}
      else 
	;//busywait when we have predicted on all examples but not yet trained on all.
    }

  if (output_regularizer)//need to accumulate and place the regularizer.
    {
      if(global.master_location != "")
	accumulate(global.master_location, reg, 3); //Accumulate preconditioner
      preconditioner_to_regularizer(reg,global.regularization);
    }
  ftime(&t_end_global);
  net_time = (int) (1000.0 * (t_end_global.time - t_start_global.time) + (t_end_global.millitm - t_start_global.millitm)); 
  if (!global.quiet)
    {
      cerr<<"Net time spent in communication = "<<get_comm_time()/(float)1000<<" seconds\n";
      cerr<<"Net time spent = "<<(float)net_time/(float)1000<<" seconds\n";
    }

  if (global.local_prediction > 0)
    shutdown(global.local_prediction, SHUT_WR);
  free(predictions.begin);
  free(mem);
  free(rho);
  free(alpha);
  free(ec);
  t.reg = reg;
}

void destroy_bfgs()
{
}

}

/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license

The algorithm here is generally based on Nocedal 1980, Liu and Nocedal 1989.
Implementation by Miro Dudik.
 */
#include <fstream>
#include <float.h>
#ifndef _WIN32
#include <netdb.h>
#endif
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sys/timeb.h>
#include "parse_example.h"
#include "constant.h"
#include "sparse_dense.h"
#include "bfgs.h"
#include "cache.h"
#include "simple_label.h"
#include "accumulate.h"
#include <exception>

#ifdef _WIN32
inline bool isnan(double d) { return (bool)_isnan(d); }
#endif

using namespace std;

#define CG_EXTRA 1

#define MEM_GT 0
#define MEM_XT 1
#define MEM_YT 0
#define MEM_ST 1

#define W_XT 0
#define W_GT 1
#define W_DIR 2
#define W_COND 3

#define LEARN_OK 0
#define LEARN_CURV 1
#define LEARN_CONV 2

class curv_exception: public exception {} curv_ex;

/********************************************************************/
/* mem & w definition ***********************************************/
/********************************************************************/ 
// mem[2*i] = y_t
// mem[2*i+1] = s_t
//
// w[0] = weight
// w[1] = accumulated first derivative
// w[2] = step direction
// w[3] = preconditioner
  
namespace BFGS 

{

 //nonrentrant

double wolfe1_bound = 0.01;

struct timeb t_start, t_end;
double net_comm_time = 0.0;

struct timeb t_start_global, t_end_global;
double net_time;

v_array<float> predictions;
size_t example_number=0;
size_t current_pass = 0;

  // default transition behavior
bool first_hessian_on=true;
bool backstep_on=false; 

  // set by initializer
int mem_stride;
bool output_regularizer;
float* mem;
double* rho;
double* alpha;

  // the below needs to be included when resetting, in addition to preconditioner and derivative
  int lastj, origin;
  double loss_sum, previous_loss_sum;
  float step_size;
  double importance_weight_sum;
  double curvature;

  // first pass specification
  bool first_pass=true;
bool gradient_pass=true;
bool preconditioner_pass=true;

const char* curv_message = "Zero or negative curvature detected.\n"
      "To increase curvature you can increase regularization or rescale features.\n"
      "It is also possible that you have reached numerical accuracy\n"
      "and further decrease in the objective cannot be reliably detected.\n";

void zero_derivative(vw& all)
{//set derivative to 0.
  uint32_t length = 1 << all.num_bits;
  size_t stride = all.stride;
  weight* weights = all.reg.weight_vectors;
  for(uint32_t i = 0; i < length; i++)
    weights[stride*i+1] = 0;
}

void zero_preconditioner(vw& all)
{//set derivative to 0.
  uint32_t length = 1 << all.num_bits;
  size_t stride = all.stride;
  weight* weights = all.reg.weight_vectors;
  for(uint32_t i = 0; i < length; i++)
    weights[stride*i+3] = 0;
}

  void reset_state(vw& all, bool zero)
  {
    lastj = origin = 0;
    loss_sum = previous_loss_sum = 0.;
    importance_weight_sum = 0.;
    curvature = 0.;
    first_pass = true;
    gradient_pass = true;
    preconditioner_pass = true;
    if (zero)
      {
	zero_derivative(all);
	zero_preconditioner(all);
      }
  }

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

bool test_example(example* ec)
{
  return ((label_data*)ec->ld)->label == FLT_MAX;
}

  float bfgs_predict(vw& all, example* &ec)
  {
    ec->partial_prediction = inline_predict(all,ec);
    return finalize_prediction(all, ec->partial_prediction);
  }

float predict_and_gradient(vw& all, example* &ec)
{
  float fp = bfgs_predict(all, ec);

  label_data* ld = (label_data*)ec->ld;
  all.set_minmax(all.sd, ld->label);

  float loss_grad = all.loss->first_derivative(all.sd, fp,ld->label)*ld->weight;
  
  size_t mask = all.weight_mask;
  weight* weights = all.reg.weight_vectors;
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
    {
      feature *f = ec->atomics[*i].begin;
      for (; f != ec->atomics[*i].end; f++)
	{
	  weight* w = &weights[f->weight_index & mask];
	  w[1] += loss_grad * f->x;
	}
    }
  for (vector<string>::iterator i = all.pairs.begin(); i != all.pairs.end();i++) 
    {
      if (ec->atomics[(int)(*i)[0]].index() > 0)
	{
	  v_array<feature> temp = ec->atomics[(int)(*i)[0]];
	  for (; temp.begin != temp.end; temp.begin++)
	    quad_grad_update(weights, *temp.begin, ec->atomics[(int)(*i)[1]], mask, loss_grad);
	} 
    }
  return fp;
}

void update_preconditioner(vw& all, example* &ec)
{
  label_data* ld = (label_data*)ec->ld;
  float curvature = all.loss->second_derivative(all.sd, ec->final_prediction,ld->label) * ld->weight;
  
  size_t mask = all.weight_mask;
  weight* weights = all.reg.weight_vectors;
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++)
    {
      feature *f = ec->atomics[*i].begin;
      for (; f != ec->atomics[*i].end; f++)
        {
          weight* w = &weights[f->weight_index & mask];
          w[3] += f->x * f->x * curvature;
        }
    }
  for (vector<string>::iterator i = all.pairs.begin(); i != all.pairs.end();i++)
    {
      if (ec->atomics[(int)(*i)[0]].index() > 0)
        {
          v_array<feature> temp = ec->atomics[(int)(*i)[0]];
          for (; temp.begin != temp.end; temp.begin++)
            quad_precond_update(weights, *temp.begin, ec->atomics[(int)(*i)[1]], mask, curvature);
        }
    }
}  


float dot_with_direction(vw& all, example* &ec)
{
  float ret = 0;
  weight* weights = all.reg.weight_vectors;
  size_t mask = all.weight_mask;
  weights +=2;//direction vector stored two advanced
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
    {
      feature *f = ec->atomics[*i].begin;
      for (; f != ec->atomics[*i].end; f++)
	ret += weights[f->weight_index & mask] * f->x;
    }
  for (vector<string>::iterator i = all.pairs.begin(); i != all.pairs.end();i++) 
    {
      if (ec->atomics[(int)(*i)[0]].index() > 0)
	{
	  v_array<feature> temp = ec->atomics[(int)(*i)[0]];
	  for (; temp.begin != temp.end; temp.begin++)
	    ret += one_pf_quad_predict(weights, *temp.begin, ec->atomics[(int)(*i)[1]], mask);
	} 
    }
  return ret;
}

double regularizer_direction_magnitude(vw& all, float regularizer)
{//compute direction magnitude
  double ret = 0.;
  
  if (regularizer == 0.)
    return ret;

  uint32_t length = 1 << all.num_bits;
  size_t stride = all.stride;
  weight* weights = all.reg.weight_vectors;
  if (all.reg.regularizers == NULL)
    for(uint32_t i = 0; i < length; i++)
      ret += regularizer*weights[stride*i+2]*weights[stride*i+2];
  else
    for(uint32_t i = 0; i < length; i++) 
      ret += all.reg.regularizers[2*i]*weights[stride*i+2]*weights[stride*i+2];

  return ret;
}

double direction_magnitude(vw& all)
{//compute direction magnitude
  double ret = 0.;
  uint32_t length = 1 << all.num_bits;
  size_t stride = all.stride;
  weight* weights = all.reg.weight_vectors;
  for(uint32_t i = 0; i < length; i++)
    ret += weights[stride*i+2]*weights[stride*i+2];
  
  return ret;
}

  void bfgs_iter_start(vw& all, float* mem, int& lastj, double importance_weight_sum, int&origin)
{
  uint32_t length = 1 << all.num_bits;
  size_t stride = all.stride;
  weight* w = all.reg.weight_vectors;

  double g1_Hg1 = 0.;
  double g1_g1 = 0.;
  
  origin = 0;
  for(uint32_t i = 0; i < length; i++, mem+=mem_stride, w+=stride) {
    if (all.m>0)
      mem[(MEM_XT+origin)%mem_stride] = w[W_XT]; 
    mem[(MEM_GT+origin)%mem_stride] = w[W_GT];
    g1_Hg1 += w[W_GT] * w[W_GT] * w[W_COND];
    g1_g1 += w[W_GT] * w[W_GT];
    w[W_DIR] = -w[W_COND]*w[W_GT];
    w[W_GT] = 0;
  }
  lastj = 0;
  if (!all.quiet)
    fprintf(stderr, "%-10e\t%-10e\t%-10s\t%-10s\t%-10s\t",
	    g1_g1/(importance_weight_sum*importance_weight_sum),
	    g1_Hg1/importance_weight_sum, "", "", "");
}

void bfgs_iter_middle(vw& all, float* mem, double* rho, double* alpha, int& lastj, int &origin) throw (curv_exception)
{  
  uint32_t length = 1 << all.num_bits;
  size_t stride = all.stride;
  weight* w = all.reg.weight_vectors;
  
  float* mem0 = mem;
  float* w0 = w;

  // implement conjugate gradient
  if (all.m==0) {
    double g_Hy = 0.;
    double g_Hg = 0.;
    double y = 0.;
  
    for(uint32_t i = 0; i < length; i++, mem+=mem_stride, w+=stride) {
      y = w[W_GT]-mem[(MEM_GT+origin)%mem_stride];
      g_Hy += w[W_GT] * w[W_COND] * y;
      g_Hg += mem[(MEM_GT+origin)%mem_stride] * w[W_COND] * mem[(MEM_GT+origin)%mem_stride];
    }

    float beta = (float) (g_Hy/g_Hg);

    if (beta<0.f || nanpattern(beta))
      beta = 0.f;
      
    mem = mem0;
    w = w0;
    for(uint32_t i = 0; i < length; i++, mem+=mem_stride, w+=stride) {
      mem[(MEM_GT+origin)%mem_stride] = w[W_GT];

      w[W_DIR] *= beta;
      w[W_DIR] -= w[W_COND]*w[W_GT];
      w[W_GT] = 0;
    }
    if (!all.quiet)
      fprintf(stderr, "%f\t", beta);
    return;
  }
  else {
    if (!all.quiet)
      fprintf(stderr, "%-10s\t","");
  }

  // implement bfgs
  double y_s = 0.;
  double y_Hy = 0.;
  double s_q = 0.;
  
  for(uint32_t i = 0; i < length; i++, mem+=mem_stride, w+=stride) {
    mem[(MEM_YT+origin)%mem_stride] = w[W_GT] - mem[(MEM_GT+origin)%mem_stride];
    mem[(MEM_ST+origin)%mem_stride] = w[W_XT] - mem[(MEM_XT+origin)%mem_stride];
    w[W_DIR] = w[W_GT];
    y_s += mem[(MEM_YT+origin)%mem_stride]*mem[(MEM_ST+origin)%mem_stride];
    y_Hy += mem[(MEM_YT+origin)%mem_stride]*mem[(MEM_YT+origin)%mem_stride]*w[W_COND];
    s_q += mem[(MEM_ST+origin)%mem_stride]*w[W_GT];  
  }
  
  if (y_s <= 0. || y_Hy <= 0.)
    throw curv_ex;
  rho[0] = 1/y_s;
  
  float gamma = (float) (y_s/y_Hy);

  for (int j=0; j<lastj; j++) {
    alpha[j] = rho[j] * s_q;
    s_q = 0.;
    mem = mem0;
    w = w0;
    for(uint32_t i = 0; i < length; i++, mem+=mem_stride, w+=stride) {
      w[W_DIR] -= (float)alpha[j]*mem[(2*j+MEM_YT+origin)%mem_stride];
      s_q += mem[(2*j+2+MEM_ST+origin)%mem_stride]*w[W_DIR];
    }
  }

  alpha[lastj] = rho[lastj] * s_q;
  double y_r = 0.;  
  mem = mem0;
  w = w0;
  for(uint32_t i = 0; i < length; i++, mem+=mem_stride, w+=stride) {
    w[W_DIR] -= (float)alpha[lastj]*mem[(2*lastj+MEM_YT+origin)%mem_stride];
    w[W_DIR] *= gamma*w[W_COND];
    y_r += mem[(2*lastj+MEM_YT+origin)%mem_stride]*w[W_DIR];
  }

  double coef_j;
    
  for (int j=lastj; j>0; j--) {
    coef_j = alpha[j] - rho[j] * y_r;
    y_r = 0.;
    mem = mem0;
    w = w0;
    for(uint32_t i = 0; i < length; i++, mem+=mem_stride, w+=stride) {
      w[W_DIR] += (float)coef_j*mem[(2*j+MEM_ST+origin)%mem_stride];
      y_r += mem[(2*j-2+MEM_YT+origin)%mem_stride]*w[W_DIR];
    }
  }

  coef_j = alpha[0] - rho[0] * y_r;
  mem = mem0;
  w = w0;
  for(uint32_t i = 0; i < length; i++, mem+=mem_stride, w+=stride) {
    w[W_DIR] = -w[W_DIR]-(float)coef_j*mem[(MEM_ST+origin)%mem_stride];
  }
  
  /*********************
   ** shift 
   ********************/

  mem = mem0;
  w = w0;
  lastj = (lastj<all.m-1) ? lastj+1 : all.m-1;
  origin = (origin+mem_stride-2)%mem_stride;
  for(uint32_t i = 0; i < length; i++, mem+=mem_stride, w+=stride) {
    mem[(MEM_GT+origin)%mem_stride] = w[W_GT];
    mem[(MEM_XT+origin)%mem_stride] = w[W_XT];
    w[W_GT] = 0;
  }
  for (int j=lastj; j>0; j--)
    rho[j] = rho[j-1];
}

double wolfe_eval(vw& all, float* mem, double loss_sum, double previous_loss_sum, double step_size, double importance_weight_sum, int &origin, double& wolfe1) { 
  uint32_t length = 1 << all.num_bits;
  size_t stride = all.stride;
  weight* w = all.reg.weight_vectors;
  
  double g0_d = 0.;
  double g1_d = 0.;
  double g1_Hg1 = 0.;
  double g1_g1 = 0.;
  
  for(uint32_t i = 0; i < length; i++, mem+=mem_stride, w+=stride) {
    g0_d += mem[(MEM_GT+origin)%mem_stride] * w[W_DIR];
    g1_d += w[W_GT] * w[W_DIR];
    g1_Hg1 += w[W_GT] * w[W_GT] * w[W_COND];
    g1_g1 += w[W_GT] * w[W_GT];
  }
  
  wolfe1 = (loss_sum-previous_loss_sum)/(step_size*g0_d);
  double wolfe2 = g1_d/g0_d;
  // double new_step_cross = (loss_sum-previous_loss_sum-g1_d*step)/(g0_d-g1_d);

  if (!all.quiet)
    fprintf(stderr, "%-10e\t%-10e\t%s%-10f\t%-10f\t", g1_g1/(importance_weight_sum*importance_weight_sum), g1_Hg1/importance_weight_sum, " ", wolfe1, wolfe2);
  return 0.5*step_size;
}


double add_regularization(vw& all, float regularization)
{//compute the derivative difference
  double ret = 0.;
  uint32_t length = 1 << all.num_bits;
  size_t stride = all.stride;
  weight* weights = all.reg.weight_vectors;
  if (all.reg.regularizers == NULL)
    {
      for(uint32_t i = 0; i < length; i++) {
	weights[stride*i+1] += regularization*weights[stride*i];
	ret += 0.5*regularization*weights[stride*i]*weights[stride*i];
      }
    }
  else
    {
      for(uint32_t i = 0; i < length; i++) {
	weight delta_weight = weights[stride*i] - all.reg.regularizers[2*i+1];
	weights[stride*i+1] += all.reg.regularizers[2*i]*delta_weight;
	ret += 0.5*all.reg.regularizers[2*i]*delta_weight*delta_weight;
      }
    }
  return ret;
}

void finalize_preconditioner(vw& all, float regularization)
{
  uint32_t length = 1 << all.num_bits;
  size_t stride = all.stride;
  weight* weights = all.reg.weight_vectors;

  if (all.reg.regularizers == NULL)
    for(uint32_t i = 0; i < length; i++) {
      weights[stride*i+3] += regularization;
      if (weights[stride*i+3] > 0)
	weights[stride*i+3] = 1.f / weights[stride*i+3];
    }
  else
    for(uint32_t i = 0; i < length; i++) {
      weights[stride*i+3] += all.reg.regularizers[2*i];
      if (weights[stride*i+3] > 0)
	weights[stride*i+3] = 1.f / weights[stride*i+3];
    }
}

void preconditioner_to_regularizer(vw& all, float regularization)
{
  uint32_t length = 1 << all.num_bits;
  size_t stride = all.stride;
  weight* weights = all.reg.weight_vectors;
  if (all.reg.regularizers == NULL)
    {
      all.reg.regularizers = (weight *)calloc(2*length, sizeof(weight));
      
      if (all.reg.regularizers == NULL)
	{
	  cerr << all.program_name << ": Failed to allocate weight array: try decreasing -b <bits>" << endl;
	  exit (1);
	}
      for(uint32_t i = 0; i < length; i++) 
	all.reg.regularizers[2*i] = weights[stride*i+3] + regularization;
    }
  else
    for(uint32_t i = 0; i < length; i++) 
      all.reg.regularizers[2*i] = weights[stride*i+3] + all.reg.regularizers[2*i];
  for(uint32_t i = 0; i < length; i++) 
    all.reg.regularizers[2*i+1] = weights[stride*i];
}

void zero_state(vw& all)
{
  uint32_t length = 1 << all.num_bits;
  size_t stride = all.stride;
  weight* weights = all.reg.weight_vectors;
  for(uint32_t i = 0; i < length; i++) 
    {
      weights[stride*i+1] = 0;
      weights[stride*i+2] = 0;
      weights[stride*i+3] = 0;
    }
}

double derivative_in_direction(vw& all, float* mem, int &origin)
  {  
  double ret = 0.;
  uint32_t length = 1 << all.num_bits;
  size_t stride = all.stride;
  weight* w = all.reg.weight_vectors;
  
  for(uint32_t i = 0; i < length; i++, w+=stride, mem+=mem_stride)
    ret += mem[(MEM_GT+origin)%mem_stride]*w[W_DIR];
  return ret;
}
  
void update_weight(vw& all, string& reg_name, float step_size, size_t current_pass)
  {
    uint32_t length = 1 << all.num_bits;
    size_t stride = all.stride;
    weight* w = all.reg.weight_vectors;
    
    for(uint32_t i = 0; i < length; i++, w+=stride)
      w[W_XT] += step_size * w[W_DIR];
    save_predictor(all, reg_name, current_pass);
  }

int process_pass(vw& all) {
  int status = LEARN_OK;

  /********************************************************************/
  /* A) FIRST PASS FINISHED: INITIALIZE FIRST LINE SEARCH *************/
  /********************************************************************/ 
    if (first_pass) {
      if(all.span_server != "")
	{
	  accumulate(all, all.span_server, all.reg, 3); //Accumulate preconditioner
	  importance_weight_sum = accumulate_scalar(all, all.span_server, importance_weight_sum);
	}
      finalize_preconditioner(all, all.l2_lambda);
      if(all.span_server != "") {
	loss_sum = accumulate_scalar(all, all.span_server, loss_sum);  //Accumulate loss_sums
	accumulate(all, all.span_server, all.reg, 1); //Accumulate gradients from all nodes
      }
      if (all.l2_lambda > 0.)
	loss_sum += add_regularization(all, all.l2_lambda);
      if (!all.quiet)
	fprintf(stderr, "%2lu %-e\t", (long unsigned int)current_pass+1, loss_sum / importance_weight_sum);
      
      previous_loss_sum = loss_sum;
      loss_sum = 0.;
      example_number = 0;
      curvature = 0;
      bfgs_iter_start(all, mem, lastj, importance_weight_sum, origin);
      if (first_hessian_on) {
	gradient_pass = false;//now start computing curvature
      }
      else {
	step_size = 0.5;
	float d_mag = direction_magnitude(all);
	ftime(&t_end_global);
	net_time = (int) (1000.0 * (t_end_global.time - t_start_global.time) + (t_end_global.millitm - t_start_global.millitm)); 
	if (!all.quiet)
	  fprintf(stderr, "%-10s\t%-10e\t%-10e\t%-10.3f\n", "", d_mag, step_size, (net_time/1000.));
	predictions.erase();
	update_weight(all, all.final_regressor_name, step_size, current_pass);		     		           }
    }
    else
  /********************************************************************/
  /* B) GRADIENT CALCULATED *******************************************/
  /********************************************************************/ 
	      if (gradient_pass) // We just finished computing all gradients
		{
		  if(all.span_server != "") {
		    loss_sum = accumulate_scalar(all, all.span_server, loss_sum);  //Accumulate loss_sums
		    accumulate(all, all.span_server, all.reg, 1); //Accumulate gradients from all nodes
		  }
		  if (all.l2_lambda > 0.)
		    loss_sum += add_regularization(all, all.l2_lambda);
		  if (!all.quiet)
		    fprintf(stderr, "%2lu %-e\t", (long unsigned int)current_pass+1, loss_sum / importance_weight_sum);

		  double wolfe1;
		  double new_step = wolfe_eval(all, mem, loss_sum, previous_loss_sum, step_size, importance_weight_sum, origin, wolfe1);

  /********************************************************************/
  /* B0) DERIVATIVE ZERO: MINIMUM FOUND *******************************/
  /********************************************************************/ 
		  if (nanpattern(wolfe1))
		    {
		      fprintf(stderr, "\n");
		      fprintf(stdout, "Derivative 0 detected.\n");
		      step_size=0.0;
		      status = LEARN_CONV;
		    }
  /********************************************************************/
  /* B1) LINE SEARCH FAILED *******************************************/
  /********************************************************************/ 
		  else if (backstep_on && (wolfe1<wolfe1_bound || loss_sum > previous_loss_sum))
		    {// curvature violated, or we stepped too far last time: step back
		      ftime(&t_end_global);
		      net_time = (int) (1000.0 * (t_end_global.time - t_start_global.time) + (t_end_global.millitm - t_start_global.millitm)); 
		      float ratio = (step_size==0.) ? 0. : new_step/step_size;
		      if (!all.quiet)
			fprintf(stderr, "%-10s\t%-10s\t(revise x %.1f)\t%-10e\t%-.3f\n",
				"","",ratio,
				new_step,
				net_time/1000.);
			predictions.erase();
			update_weight(all, all.final_regressor_name, -step_size+new_step, current_pass);		     		      			
			step_size = new_step;
			zero_derivative(all);
			loss_sum = 0.;
		    }

  /********************************************************************/
  /* B2) LINE SEARCH SUCCESSFUL OR DISABLED          ******************/
  /*     DETERMINE NEXT SEARCH DIRECTION             ******************/
  /********************************************************************/ 
		  else {
		      double rel_decrease = (previous_loss_sum-loss_sum)/previous_loss_sum;
		      if (!nanpattern(rel_decrease) && backstep_on && fabs(rel_decrease)<all.rel_threshold) {
			fprintf(stdout, "\nTermination condition reached in pass %ld: decrease in loss less than %.3f%%.\n"
				"If you want to optimize further, decrease termination threshold.\n", (long int)current_pass+1, all.rel_threshold*100.0);
			status = LEARN_CONV;
		      }
		      previous_loss_sum = loss_sum;
		      loss_sum = 0.;
		      example_number = 0;
		      curvature = 0;
		      step_size = 1.0;

		      try {
		      bfgs_iter_middle(all, mem, rho, alpha, lastj, origin);
		      }
		      catch (curv_exception e) {
			fprintf(stdout, "In bfgs_iter_middle: %s", curv_message);
			step_size=0.0;
			status = LEARN_CURV;
		      }

		      if (all.hessian_on) {
			gradient_pass = false;//now start computing curvature
		      }
		      else {
			float d_mag = direction_magnitude(all);
			ftime(&t_end_global);
			net_time = (int) (1000.0 * (t_end_global.time - t_start_global.time) + (t_end_global.millitm - t_start_global.millitm)); 
			if (!all.quiet)
			  fprintf(stderr, "%-10s\t%-10e\t%-10e\t%-10.3f\n", "", d_mag, step_size, (net_time/1000.));
			predictions.erase();
			update_weight(all, all.final_regressor_name, step_size, current_pass);		     		      
		      }
		    }
		}

  /********************************************************************/
  /* C) NOT FIRST PASS, CURVATURE CALCULATED **************************/
  /********************************************************************/ 
	      else // just finished all second gradients
		{
		  if(all.span_server != "") {
		    curvature = accumulate_scalar(all, all.span_server, curvature);  //Accumulate curvatures
		  }
		  if (all.l2_lambda > 0.)
		    curvature += regularizer_direction_magnitude(all, all.l2_lambda);
		  float dd = derivative_in_direction(all, mem, origin);
		  if (curvature == 0. && dd != 0.)
		    {
		      fprintf(stdout, "%s", curv_message);
		      step_size=0.0;
		      status = LEARN_CURV;
		    }
		  else if ( dd == 0.)
		    {
		      fprintf(stdout, "Derivative 0 detected.\n");
		      step_size=0.0;
		      status = LEARN_CONV;
		    }
		  else
		    step_size = - dd/curvature;
		  
		  float d_mag = direction_magnitude(all);

		  predictions.erase();
		  update_weight(all, all.final_regressor_name , step_size, current_pass);
		  ftime(&t_end_global);
		  net_time = (int) (1000.0 * (t_end_global.time - t_start_global.time) + (t_end_global.millitm - t_start_global.millitm)); 
		  if (!all.quiet)
		    fprintf(stderr, "%-e\t%-e\t%-e\t%-.3f\n", curvature / importance_weight_sum, d_mag, step_size,(net_time/1000.));
		  gradient_pass = true;
		}//now start computing derivatives.    
    current_pass++;
    first_pass = false;
    preconditioner_pass = false;
    return status;
}

void process_example(vw& all, example *ec)
 {
  label_data* ld = (label_data*)ec->ld;
  if (first_pass)
    importance_weight_sum += ld->weight;
  
  /********************************************************************/
  /* I) GRADIENT CALCULATION ******************************************/
  /********************************************************************/ 
  if (gradient_pass)
    {
      ec->final_prediction = predict_and_gradient(all, ec);//w[0] & w[1]
      ec->loss = all.loss->getLoss(all.sd, ec->final_prediction, ld->label) * ld->weight;
      loss_sum += ec->loss;
      push(predictions,ec->final_prediction);
    }
  /********************************************************************/
  /* II) CURVATURE CALCULATION ****************************************/
  /********************************************************************/ 
  else //computing curvature
    {
      float d_dot_x = dot_with_direction(all, ec);//w[2]
      if (example_number >= predictions.index())//Make things safe in case example source is strange.
	example_number = predictions.index()-1;
      ec->final_prediction = predictions[example_number];
      ec->partial_prediction = predictions[example_number];
      ec->loss = all.loss->getLoss(all.sd, ec->final_prediction, ld->label) * ld->weight;	      
      float sd = all.loss->second_derivative(all.sd, predictions[example_number++],ld->label);
      curvature += d_dot_x*d_dot_x*sd*ld->weight;
    }
  
  if (preconditioner_pass)
    update_preconditioner(all, ec);//w[3]
 }

void learn(void* a, example* ec)
{
  vw* all = (vw*)a;
  assert(ec->in_use);
  if (ec->pass != current_pass) {
    int status = process_pass(*all);
    if (status != LEARN_OK)
      reset_state(*all, true);
    else if (output_regularizer && current_pass==all->numpasses-1) {
      zero_preconditioner(*all);
      preconditioner_pass = true;
    }
  }
  if (test_example(ec))
    ec->final_prediction = bfgs_predict(*all,ec);//w[0]
  else
    process_example(*all, ec);
}

void finish(void* a)
{
  vw* all = (vw*)a;
  if (current_pass != 0 && !output_regularizer)
    process_pass(*all);
  if (!all->quiet)
    fprintf(stderr, "\n");

  if (output_regularizer)//need to accumulate and place the regularizer.
    {
      if(all->span_server != "")
	accumulate(*all, all->span_server, all->reg, 3); //Accumulate preconditioner
      preconditioner_to_regularizer(*all, all->l2_lambda);
    }
  ftime(&t_end_global);
  net_time = (int) (1000.0 * (t_end_global.time - t_start_global.time) + (t_end_global.millitm - t_start_global.millitm)); 

  free(predictions.begin);
  free(mem);
  free(rho);
  free(alpha);
}

void initializer(vw& all)
{
  int m = all.m;

  mem_stride = (m==0) ? CG_EXTRA : 2*m;
  mem = (float*) malloc(sizeof(float)*all.length()*(mem_stride));
  rho = (double*) malloc(sizeof(double)*m);
  alpha = (double*) malloc(sizeof(double)*m);

  if (!all.quiet) 
    {
      fprintf(stderr, "m = %d\nAllocated %luM for weights and mem\n", m, (long unsigned int)all.length()*(sizeof(float)*(mem_stride)+sizeof(weight)*all.stride) >> 20);
    }

  net_time = 0.0;
  ftime(&t_start_global);
  
  if (!all.quiet)
    {
      const char * header_fmt = "%2s %-10s\t%-10s\t%-10s\t %-10s\t%-10s\t%-10s\t%-10s\t%-10s\t%-10s\t%-10s\n";
      fprintf(stderr, header_fmt,
	      "##", "avg. loss", "der. mag.", "d. m. cond.", "wolfe1", "wolfe2", "mix fraction", "curvature", "dir. magnitude", "step size", "time");
      cerr.precision(5);
    }

  if (all.reg.regularizers != NULL)
      all.l2_lambda = 1; // To make sure we are adding the regularization
  output_regularizer =  (all.per_feature_regularizer_output != "" || all.per_feature_regularizer_text != "");
  reset_state(all, false);
}

void drive_bfgs(void* in)
{
  vw* all = (vw*)in;

  example* ec = NULL;

  size_t final_pass=all->numpasses-1;
  first_hessian_on = true;
  backstep_on = true;

  while ( true )
    {
      if ((ec = get_example(all->p)) != NULL)//semiblocking operation.
	{
	  assert(ec->in_use);	  

	  if (ec->pass<=final_pass) {
	    if (ec->pass != current_pass) {
	      int status = process_pass(*all);
	      if (status != LEARN_OK && final_pass>current_pass) {
		final_pass = current_pass;
	      }
	      if (output_regularizer && current_pass==final_pass) {
		zero_preconditioner(*all);
		preconditioner_pass = true;
	      }
	    }
	    process_example(*all, ec);
	  }

	  finish_example(*all, ec);
	}
     else if (parser_done(all->p))
	{
          //	  finish(all);
	  return;
	}
      else 
	;//busywait when we have predicted on all examples but not yet trained on all.
    }
}

}

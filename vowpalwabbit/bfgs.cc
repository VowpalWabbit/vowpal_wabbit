/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
/*
The algorithm here is generally based on Nocedal 1980, Liu and Nocedal 1989.
Implementation by Miro Dudik.
 */
#include <fstream>
#include <float.h>
#include <exception>
#ifndef _WIN32
#include <netdb.h>
#endif
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sys/timeb.h>
#include "constant.h"
#include "simple_label.h"
#include "accumulate.h"
#include "vw.h"
#include "gd.h"
#include "reductions.h"

using namespace std;
using namespace LEARNER;

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
  const float max_precond_ratio = 100.f;

  struct bfgs {
    vw* all;
    double wolfe1_bound;
    
    size_t final_pass;
    struct timeb t_start, t_end;
    double net_comm_time;
    
    struct timeb t_start_global, t_end_global;
    double net_time;
    
    v_array<float> predictions;
    size_t example_number;
    size_t current_pass;
    size_t no_win_counter;
    size_t early_stop_thres;
    
    // default transition behavior
    bool first_hessian_on;
    bool backstep_on;
    
    // set by initializer
    int mem_stride;
    bool output_regularizer;
    float* mem;
    double* rho;
    double* alpha;
    
    weight* regularizers;
    // the below needs to be included when resetting, in addition to preconditioner and derivative
    int lastj, origin;
    double loss_sum, previous_loss_sum;
    float step_size;
    double importance_weight_sum;
    double curvature;
    
    // first pass specification
    bool first_pass;
    bool gradient_pass;
    bool preconditioner_pass;
  };

const char* curv_message = "Zero or negative curvature detected.\n"
      "To increase curvature you can increase regularization or rescale features.\n"
      "It is also possible that you have reached numerical accuracy\n"
      "and further decrease in the objective cannot be reliably detected.\n";

void zero_derivative(vw& all)
{//set derivative to 0.
  uint32_t length = 1 << all.num_bits;
  size_t stride_shift = all.reg.stride_shift;
  weight* weights = all.reg.weight_vector;
  for(uint32_t i = 0; i < length; i++)
    weights[(i << stride_shift) +W_GT] = 0;
}

void zero_preconditioner(vw& all)
{//set derivative to 0.
  uint32_t length = 1 << all.num_bits;
  size_t stride_shift = all.reg.stride_shift;
  weight* weights = all.reg.weight_vector;
  for(uint32_t i = 0; i < length; i++)
    weights[(i << stride_shift)+W_COND] = 0;
}

void reset_state(vw& all, bfgs& b, bool zero)
{
  b.lastj = b.origin = 0;
  b.loss_sum = b.previous_loss_sum = 0.;
  b.importance_weight_sum = 0.;
  b.curvature = 0.;
  b.first_pass = true;
  b.gradient_pass = true;
  b.preconditioner_pass = true;
  if (zero)
    {
      zero_derivative(all);
      zero_preconditioner(all);
    }
}

// w[0] = weight
// w[1] = accumulated first derivative
// w[2] = step direction
// w[3] = preconditioner

bool test_example(example& ec)
{
  return ((label_data*)ec.ld)->label == FLT_MAX;
}

  float bfgs_predict(vw& all, example& ec)
  {
    ec.partial_prediction = GD::inline_predict<vec_add>(all,ec);
    return GD::finalize_prediction(all, ec.partial_prediction);
  }

inline void add_grad(float& d, float f, float& fw)
{
  fw += d * f;
}

float predict_and_gradient(vw& all, example &ec)
{
  float fp = bfgs_predict(all, ec);

  label_data* ld = (label_data*)ec.ld;
  all.set_minmax(all.sd, ld->label);

  float loss_grad = all.loss->first_derivative(all.sd, fp,ld->label)*ld->weight;
  
  ec.ft_offset += W_GT;
  GD::foreach_feature<float,add_grad>(all, ec, loss_grad);
  ec.ft_offset -= W_GT;
  
  return fp;
}

inline void add_precond(float& d, float f, float& fw)
{
  fw += d * f * f;
}

void update_preconditioner(vw& all, example& ec)
{
  label_data* ld = (label_data*)ec.ld;
  float curvature = all.loss->second_derivative(all.sd, ec.final_prediction,ld->label) * ld->weight;
  
  ec.ft_offset += W_COND;
  GD::foreach_feature<float,add_precond>(all, ec, curvature);  
  ec.ft_offset -= W_COND;
}  


float dot_with_direction(vw& all, example& ec)
{
  ec.ft_offset+= W_DIR;  
  float ret = GD::inline_predict<vec_add>(all, ec);
  ec.ft_offset-= W_DIR;

  return ret;
}

double regularizer_direction_magnitude(vw& all, bfgs& b, float regularizer)
{//compute direction magnitude
  double ret = 0.;
  
  if (regularizer == 0.)
    return ret;

  uint32_t length = 1 << all.num_bits;
  size_t stride_shift = all.reg.stride_shift;
  weight* weights = all.reg.weight_vector;
  if (b.regularizers == NULL)
    for(uint32_t i = 0; i < length; i++)
      ret += regularizer*weights[(i << stride_shift)+W_DIR]*weights[(i << stride_shift)+W_DIR];
  else
    for(uint32_t i = 0; i < length; i++) 
      ret += b.regularizers[2*i]*weights[(i << stride_shift)+W_DIR]*weights[(i << stride_shift)+W_DIR];

  return ret;
}

float direction_magnitude(vw& all)
{//compute direction magnitude
  double ret = 0.;
  uint32_t length = 1 << all.num_bits;
  size_t stride_shift = all.reg.stride_shift;
  weight* weights = all.reg.weight_vector;
  for(uint32_t i = 0; i < length; i++)
    ret += weights[(i << stride_shift)+W_DIR]*weights[(i << stride_shift)+W_DIR];
  
  return (float)ret;
}

void bfgs_iter_start(vw& all, bfgs& b, float* mem, int& lastj, double importance_weight_sum, int&origin)
{
  uint32_t length = 1 << all.num_bits;
  size_t stride = 1 << all.reg.stride_shift;
  weight* w = all.reg.weight_vector;

  double g1_Hg1 = 0.;
  double g1_g1 = 0.;
  
  origin = 0;
  for(uint32_t i = 0; i < length; i++, mem+=b.mem_stride, w+=stride) {
    if (all.m>0)
      mem[(MEM_XT+origin)%b.mem_stride] = w[W_XT]; 
    mem[(MEM_GT+origin)%b.mem_stride] = w[W_GT];
    g1_Hg1 += w[W_GT] * w[W_GT] * w[W_COND];
    g1_g1 += w[W_GT] * w[W_GT];
    w[W_DIR] = -w[W_COND]*w[W_GT];
    w[W_GT] = 0;
  }
  lastj = 0;
  if (!all.quiet)
    fprintf(stderr, "%-10.5f\t%-10.5f\t%-10s\t%-10s\t%-10s\t",
	    g1_g1/(importance_weight_sum*importance_weight_sum),
	    g1_Hg1/importance_weight_sum, "", "", "");
}

void bfgs_iter_middle(vw& all, bfgs& b, float* mem, double* rho, double* alpha, int& lastj, int &origin) 
{  
  uint32_t length = 1 << all.num_bits;
  size_t stride = 1 << all.reg.stride_shift;
  weight* w = all.reg.weight_vector;
  
  float* mem0 = mem;
  float* w0 = w;

  // implement conjugate gradient
  if (all.m==0) {
    double g_Hy = 0.;
    double g_Hg = 0.;
    double y = 0.;
  
    for(uint32_t i = 0; i < length; i++, mem+=b.mem_stride, w+=stride) {
      y = w[W_GT]-mem[(MEM_GT+origin)%b.mem_stride];
      g_Hy += w[W_GT] * w[W_COND] * y;
      g_Hg += mem[(MEM_GT+origin)%b.mem_stride] * w[W_COND] * mem[(MEM_GT+origin)%b.mem_stride];
    }

    float beta = (float) (g_Hy/g_Hg);

    if (beta<0.f || nanpattern(beta))
      beta = 0.f;
      
    mem = mem0;
    w = w0;
    for(uint32_t i = 0; i < length; i++, mem+=b.mem_stride, w+=stride) {
      mem[(MEM_GT+origin)%b.mem_stride] = w[W_GT];

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
  
  for(uint32_t i = 0; i < length; i++, mem+=b.mem_stride, w+=stride) {
    mem[(MEM_YT+origin)%b.mem_stride] = w[W_GT] - mem[(MEM_GT+origin)%b.mem_stride];
    mem[(MEM_ST+origin)%b.mem_stride] = w[W_XT] - mem[(MEM_XT+origin)%b.mem_stride];
    w[W_DIR] = w[W_GT];
    y_s += mem[(MEM_YT+origin)%b.mem_stride]*mem[(MEM_ST+origin)%b.mem_stride];
    y_Hy += mem[(MEM_YT+origin)%b.mem_stride]*mem[(MEM_YT+origin)%b.mem_stride]*w[W_COND];
    s_q += mem[(MEM_ST+origin)%b.mem_stride]*w[W_GT];  
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
    for(uint32_t i = 0; i < length; i++, mem+=b.mem_stride, w+=stride) {
      w[W_DIR] -= (float)alpha[j]*mem[(2*j+MEM_YT+origin)%b.mem_stride];
      s_q += mem[(2*j+2+MEM_ST+origin)%b.mem_stride]*w[W_DIR];
    }
  }

  alpha[lastj] = rho[lastj] * s_q;
  double y_r = 0.;  
  mem = mem0;
  w = w0;
  for(uint32_t i = 0; i < length; i++, mem+=b.mem_stride, w+=stride) {
    w[W_DIR] -= (float)alpha[lastj]*mem[(2*lastj+MEM_YT+origin)%b.mem_stride];
    w[W_DIR] *= gamma*w[W_COND];
    y_r += mem[(2*lastj+MEM_YT+origin)%b.mem_stride]*w[W_DIR];
  }

  double coef_j;
    
  for (int j=lastj; j>0; j--) {
    coef_j = alpha[j] - rho[j] * y_r;
    y_r = 0.;
    mem = mem0;
    w = w0;
    for(uint32_t i = 0; i < length; i++, mem+=b.mem_stride, w+=stride) {
      w[W_DIR] += (float)coef_j*mem[(2*j+MEM_ST+origin)%b.mem_stride];
      y_r += mem[(2*j-2+MEM_YT+origin)%b.mem_stride]*w[W_DIR];
    }
  }


  coef_j = alpha[0] - rho[0] * y_r;
  mem = mem0;
  w = w0;
  for(uint32_t i = 0; i < length; i++, mem+=b.mem_stride, w+=stride) {
    w[W_DIR] = -w[W_DIR]-(float)coef_j*mem[(MEM_ST+origin)%b.mem_stride];
  }
  
  /*********************
   ** shift 
   ********************/

  mem = mem0;
  w = w0;
  lastj = (lastj<all.m-1) ? lastj+1 : all.m-1;
  origin = (origin+b.mem_stride-2)%b.mem_stride;
  for(uint32_t i = 0; i < length; i++, mem+=b.mem_stride, w+=stride) {
    mem[(MEM_GT+origin)%b.mem_stride] = w[W_GT];
    mem[(MEM_XT+origin)%b.mem_stride] = w[W_XT];
    w[W_GT] = 0;
  }
  for (int j=lastj; j>0; j--)
    rho[j] = rho[j-1];
}

double wolfe_eval(vw& all, bfgs& b, float* mem, double loss_sum, double previous_loss_sum, double step_size, double importance_weight_sum, int &origin, double& wolfe1) { 
  uint32_t length = 1 << all.num_bits;
  size_t stride = 1 << all.reg.stride_shift;
  weight* w = all.reg.weight_vector;
  
  double g0_d = 0.;
  double g1_d = 0.;
  double g1_Hg1 = 0.;
  double g1_g1 = 0.;
  
  for(uint32_t i = 0; i < length; i++, mem+=b.mem_stride, w+=stride) {
    g0_d += mem[(MEM_GT+origin)%b.mem_stride] * w[W_DIR];
    g1_d += w[W_GT] * w[W_DIR];
    g1_Hg1 += w[W_GT] * w[W_GT] * w[W_COND];
    g1_g1 += w[W_GT] * w[W_GT];
  }
  
  wolfe1 = (loss_sum-previous_loss_sum)/(step_size*g0_d);
  double wolfe2 = g1_d/g0_d;
  // double new_step_cross = (loss_sum-previous_loss_sum-g1_d*step)/(g0_d-g1_d);

  if (!all.quiet)
    fprintf(stderr, "%-10.5f\t%-10.5f\t%s%-10f\t%-10f\t", g1_g1/(importance_weight_sum*importance_weight_sum), g1_Hg1/importance_weight_sum, " ", wolfe1, wolfe2);
  return 0.5*step_size;
}


double add_regularization(vw& all, bfgs& b, float regularization)
{//compute the derivative difference
  double ret = 0.;
  uint32_t length = 1 << all.num_bits;
  size_t stride_shift = all.reg.stride_shift;
  weight* weights = all.reg.weight_vector;
  if (b.regularizers == NULL)
    {
      for(uint32_t i = 0; i < length; i++) {
	weights[(i << stride_shift)+W_GT] += regularization*weights[i << stride_shift];
	ret += 0.5*regularization*weights[i << stride_shift]*weights[i << stride_shift];
      }
    }
  else
    {
      for(uint32_t i = 0; i < length; i++) {
	weight delta_weight = weights[i << stride_shift] - b.regularizers[2*i+1];
	weights[(i << stride_shift)+W_GT] += b.regularizers[2*i]*delta_weight;
	ret += 0.5*b.regularizers[2*i]*delta_weight*delta_weight;
      }
    }

  return ret;
}

void finalize_preconditioner(vw& all, bfgs& b, float regularization)
{
  uint32_t length = 1 << all.num_bits;
  size_t stride = 1 << all.reg.stride_shift;
  weight* weights = all.reg.weight_vector;
  float max_hessian = 0.f;

  if (b.regularizers == NULL)
    for(uint32_t i = 0; i < length; i++) {
      weights[stride*i+W_COND] += regularization;
	  if (weights[stride*i+W_COND] > max_hessian)
		  max_hessian = weights[stride*i+W_COND];
      if (weights[stride*i+W_COND] > 0)
	weights[stride*i+W_COND] = 1.f / weights[stride*i+W_COND];
    }
  else
    for(uint32_t i = 0; i < length; i++) {
      weights[stride*i+W_COND] += b.regularizers[2*i];
	  if (weights[stride*i+W_COND] > max_hessian)
		  max_hessian = weights[stride*i+W_COND];
      if (weights[stride*i+W_COND] > 0)
	weights[stride*i+W_COND] = 1.f / weights[stride*i+W_COND];
    }

  float max_precond = (max_hessian==0.f) ? 0.f : max_precond_ratio / max_hessian;
  weights = all.reg.weight_vector;
  for(uint32_t i = 0; i < length; i++) {
    if (infpattern(weights[stride*i+W_COND]) || weights[stride*i+W_COND]>max_precond)
			weights[stride*i+W_COND] = max_precond;
  }
}

void preconditioner_to_regularizer(vw& all, bfgs& b, float regularization)
{
  uint32_t length = 1 << all.num_bits;
  size_t stride = 1 << all.reg.stride_shift;
  weight* weights = all.reg.weight_vector;
  if (b.regularizers == NULL)
    {
      b.regularizers = (weight *)calloc_or_die(2*length, sizeof(weight));
      
      if (b.regularizers == NULL)
	{
	  cerr << all.program_name << ": Failed to allocate weight array: try decreasing -b <bits>" << endl;
	  throw exception();
	}
      for(uint32_t i = 0; i < length; i++) 
	b.regularizers[2*i] = weights[stride*i+W_COND] + regularization;
    }
  else
    for(uint32_t i = 0; i < length; i++) 
      b.regularizers[2*i] = weights[stride*i+W_COND] + b.regularizers[2*i];
  for(uint32_t i = 0; i < length; i++) 
      b.regularizers[2*i+1] = weights[stride*i];
}

void zero_state(vw& all)
{
  uint32_t length = 1 << all.num_bits;
  size_t stride = 1 << all.reg.stride_shift;
  weight* weights = all.reg.weight_vector;
  for(uint32_t i = 0; i < length; i++) 
    {
      weights[stride*i+W_GT] = 0;
      weights[stride*i+W_DIR] = 0;
      weights[stride*i+W_COND] = 0;
    }
}

double derivative_in_direction(vw& all, bfgs& b, float* mem, int &origin)
  {  
  double ret = 0.;
  uint32_t length = 1 << all.num_bits;
  size_t stride = 1 << all.reg.stride_shift;
  weight* w = all.reg.weight_vector;
  
  for(uint32_t i = 0; i < length; i++, w+=stride, mem+=b.mem_stride)
    ret += mem[(MEM_GT+origin)%b.mem_stride]*w[W_DIR];
  return ret;
}
  
void update_weight(vw& all, float step_size, size_t current_pass)
  {
    uint32_t length = 1 << all.num_bits;
    size_t stride = 1 << all.reg.stride_shift;
    weight* w = all.reg.weight_vector;
    
    for(uint32_t i = 0; i < length; i++, w+=stride)
      w[W_XT] += step_size * w[W_DIR];
  }

int process_pass(vw& all, bfgs& b) {
  int status = LEARN_OK;

  /********************************************************************/
  /* A) FIRST PASS FINISHED: INITIALIZE FIRST LINE SEARCH *************/
  /********************************************************************/ 
    if (b.first_pass) {
      if(all.span_server != "")
	{
	  accumulate(all, all.span_server, all.reg, W_COND); //Accumulate preconditioner
	  float temp = (float)b.importance_weight_sum;
	  b.importance_weight_sum = accumulate_scalar(all, all.span_server, temp);
	}
      finalize_preconditioner(all, b, all.l2_lambda);
      if(all.span_server != "") {
	float temp = (float)b.loss_sum;
	b.loss_sum = accumulate_scalar(all, all.span_server, temp);  //Accumulate loss_sums
	accumulate(all, all.span_server, all.reg, 1); //Accumulate gradients from all nodes
      }
      if (all.l2_lambda > 0.)
	b.loss_sum += add_regularization(all, b, all.l2_lambda);
      if (!all.quiet)
	fprintf(stderr, "%2lu %-10.5f\t", (long unsigned int)b.current_pass+1, b.loss_sum / b.importance_weight_sum);
      
      b.previous_loss_sum = b.loss_sum;
      b.loss_sum = 0.;
      b.example_number = 0;
      b.curvature = 0;
      bfgs_iter_start(all, b, b.mem, b.lastj, b.importance_weight_sum, b.origin);
      if (b.first_hessian_on) {
	b.gradient_pass = false;//now start computing curvature
      }
      else {
	b.step_size = 0.5;
	float d_mag = direction_magnitude(all);
	ftime(&b.t_end_global);
	b.net_time = (int) (1000.0 * (b.t_end_global.time - b.t_start_global.time) + (b.t_end_global.millitm - b.t_start_global.millitm)); 
	if (!all.quiet)
	  fprintf(stderr, "%-10s\t%-10.5f\t%-10.5f\n", "", d_mag, b.step_size);
	b.predictions.erase();
	update_weight(all, b.step_size, b.current_pass);		     		           }
    }
    else
  /********************************************************************/
  /* B) GRADIENT CALCULATED *******************************************/
  /********************************************************************/ 
	      if (b.gradient_pass) // We just finished computing all gradients
		{
		  if(all.span_server != "") {
		    float t = (float)b.loss_sum;
		    b.loss_sum = accumulate_scalar(all, all.span_server, t);  //Accumulate loss_sums
		    accumulate(all, all.span_server, all.reg, 1); //Accumulate gradients from all nodes
		  }
		  if (all.l2_lambda > 0.)
		    b.loss_sum += add_regularization(all, b, all.l2_lambda);
		  if (!all.quiet){
                    if(!all.holdout_set_off && b.current_pass >= 1){
                      if(all.sd->holdout_sum_loss_since_last_pass == 0. && all.sd->weighted_holdout_examples_since_last_pass == 0.){
                        fprintf(stderr, "%2lu ", (long unsigned int)b.current_pass+1);
                        fprintf(stderr, "h unknown    ");
                      }                      
                      else
                        fprintf(stderr, "%2lu h%-10.5f\t", (long unsigned int)b.current_pass+1, all.sd->holdout_sum_loss_since_last_pass / all.sd->weighted_holdout_examples_since_last_pass);
                    }
                    else
                      fprintf(stderr, "%2lu %-10.5f\t", (long unsigned int)b.current_pass+1, b.loss_sum / b.importance_weight_sum);
                  }
		  double wolfe1;
		  double new_step = wolfe_eval(all, b, b.mem, b.loss_sum, b.previous_loss_sum, b.step_size, b.importance_weight_sum, b.origin, wolfe1);

  /********************************************************************/
  /* B0) DERIVATIVE ZERO: MINIMUM FOUND *******************************/
  /********************************************************************/ 
		  if (nanpattern((float)wolfe1))
		    {
		      fprintf(stderr, "\n");
		      fprintf(stdout, "Derivative 0 detected.\n");
		      b.step_size=0.0;
		      status = LEARN_CONV;
		    }
  /********************************************************************/
  /* B1) LINE SEARCH FAILED *******************************************/
  /********************************************************************/ 
		  else if (b.backstep_on && (wolfe1<b.wolfe1_bound || b.loss_sum > b.previous_loss_sum))
		    {// curvature violated, or we stepped too far last time: step back
		      ftime(&b.t_end_global);
		      b.net_time = (int) (1000.0 * (b.t_end_global.time - b.t_start_global.time) + (b.t_end_global.millitm - b.t_start_global.millitm)); 
		      float ratio = (b.step_size==0.f) ? 0.f : (float)new_step/(float)b.step_size;
		      if (!all.quiet)
			fprintf(stderr, "%-10s\t%-10s\t(revise x %.1f)\t%-10.5f\n",
				"","",ratio,
				new_step);
			b.predictions.erase();
			update_weight(all, (float)(-b.step_size+new_step), b.current_pass);		     		      			
			b.step_size = (float)new_step;
			zero_derivative(all);
			b.loss_sum = 0.;
		    }

  /********************************************************************/
  /* B2) LINE SEARCH SUCCESSFUL OR DISABLED          ******************/
  /*     DETERMINE NEXT SEARCH DIRECTION             ******************/
  /********************************************************************/ 
		  else {
		      double rel_decrease = (b.previous_loss_sum-b.loss_sum)/b.previous_loss_sum;
		      if (!nanpattern((float)rel_decrease) && b.backstep_on && fabs(rel_decrease)<all.rel_threshold) {
			fprintf(stdout, "\nTermination condition reached in pass %ld: decrease in loss less than %.3f%%.\n"
				"If you want to optimize further, decrease termination threshold.\n", (long int)b.current_pass+1, all.rel_threshold*100.0);
			status = LEARN_CONV;
		      }
		      b.previous_loss_sum = b.loss_sum;
		      b.loss_sum = 0.;
		      b.example_number = 0;
		      b.curvature = 0;
		      b.step_size = 1.0;

		      try {
			bfgs_iter_middle(all, b, b.mem, b.rho, b.alpha, b.lastj, b.origin);
		      }
		      catch (curv_exception e) {
			fprintf(stdout, "In bfgs_iter_middle: %s", curv_message);
			b.step_size=0.0;
			status = LEARN_CURV;
		      }

		      if (all.hessian_on) {
			b.gradient_pass = false;//now start computing curvature
		      }
		      else {
			float d_mag = direction_magnitude(all);
			ftime(&b.t_end_global);
			b.net_time = (int) (1000.0 * (b.t_end_global.time - b.t_start_global.time) + (b.t_end_global.millitm - b.t_start_global.millitm)); 
			if (!all.quiet)
			  fprintf(stderr, "%-10s\t%-10.5f\t%-10.5f\n", "", d_mag, b.step_size);
			b.predictions.erase();
			update_weight(all, b.step_size, b.current_pass);		     		      
		      }
		    }
		}

  /********************************************************************/
  /* C) NOT FIRST PASS, CURVATURE CALCULATED **************************/
  /********************************************************************/ 
	      else // just finished all second gradients
		{
		  if(all.span_server != "") {
		    float t = (float)b.curvature;
		    b.curvature = accumulate_scalar(all, all.span_server, t);  //Accumulate curvatures
		  }
		  if (all.l2_lambda > 0.)
		    b.curvature += regularizer_direction_magnitude(all, b, all.l2_lambda);
		  float dd = (float)derivative_in_direction(all, b, b.mem, b.origin);
		  if (b.curvature == 0. && dd != 0.)
		    {
		      fprintf(stdout, "%s", curv_message);
		      b.step_size=0.0;
		      status = LEARN_CURV;
		    }
		  else if ( dd == 0.)
		    {
		      fprintf(stdout, "Derivative 0 detected.\n");
		      b.step_size=0.0;
		      status = LEARN_CONV;
		    }
		  else
		    b.step_size = - dd/(float)b.curvature;
		  
		  float d_mag = direction_magnitude(all);

		  b.predictions.erase();
		  update_weight(all, b.step_size, b.current_pass);
		  ftime(&b.t_end_global);
		  b.net_time = (int) (1000.0 * (b.t_end_global.time - b.t_start_global.time) + (b.t_end_global.millitm - b.t_start_global.millitm)); 
		  if (!all.quiet)
		    fprintf(stderr, "%-10.5f\t%-10.5f\t%-10.5f\n", b.curvature / b.importance_weight_sum, d_mag, b.step_size);
		  b.gradient_pass = true;
		}//now start computing derivatives.    
    b.current_pass++;
    b.first_pass = false;
    b.preconditioner_pass = false;
    
    if (b.output_regularizer)//need to accumulate and place the regularizer.
      {
	if(all.span_server != "")
	  accumulate(all, all.span_server, all.reg, W_COND); //Accumulate preconditioner
	//preconditioner_to_regularizer(all, b, all.l2_lambda);
      }
    ftime(&b.t_end_global);
    b.net_time = (int) (1000.0 * (b.t_end_global.time - b.t_start_global.time) + (b.t_end_global.millitm - b.t_start_global.millitm)); 

    if (all.save_per_pass)
      save_predictor(all, all.final_regressor_name, b.current_pass);
    return status;
}

void process_example(vw& all, bfgs& b, example& ec)
 {
  label_data* ld = (label_data*)ec.ld;
  if (b.first_pass)
    b.importance_weight_sum += ld->weight;
  
  /********************************************************************/
  /* I) GRADIENT CALCULATION ******************************************/
  /********************************************************************/ 
  if (b.gradient_pass)
    {
      ec.final_prediction = predict_and_gradient(all, ec);//w[0] & w[1]
      ec.loss = all.loss->getLoss(all.sd, ec.final_prediction, ld->label) * ld->weight;
      b.loss_sum += ec.loss;
      b.predictions.push_back(ec.final_prediction);
    }
  /********************************************************************/
  /* II) CURVATURE CALCULATION ****************************************/
  /********************************************************************/ 
  else //computing curvature
    {
      float d_dot_x = dot_with_direction(all, ec);//w[2]
      if (b.example_number >= b.predictions.size())//Make things safe in case example source is strange.
	b.example_number = b.predictions.size()-1;
      ec.final_prediction = b.predictions[b.example_number];
      ec.partial_prediction = b.predictions[b.example_number];
      ec.loss = all.loss->getLoss(all.sd, ec.final_prediction, ld->label) * ld->weight;	      
      float sd = all.loss->second_derivative(all.sd, b.predictions[b.example_number++],ld->label);
      b.curvature += d_dot_x*d_dot_x*sd*ld->weight;
    }
  
  if (b.preconditioner_pass)
    update_preconditioner(all, ec);//w[3]
 }

void end_pass(bfgs& b)
{
  vw* all = b.all;
  
  if (b.current_pass <= b.final_pass) 
  {
       if(b.current_pass < b.final_pass)
       { 
          int status = process_pass(*all, b);

          //reaching the max number of passes regardless of convergence 
          if(b.final_pass == b.current_pass)
          {
             cerr<<"Maximum number of passes reached. ";
             if(!b.output_regularizer)
                cerr<<"If you want to optimize further, increase the number of passes\n";
             if(b.output_regularizer)
             { 
               cerr<<"\nRegular model file has been created. "; 
               cerr<<"Output feature regularizer file is created only when the convergence is reached. Try increasing the number of passes for convergence\n";
               b.output_regularizer = false;
             }

          } 
          
          //attain convergence before reaching max iterations 
	   if (status != LEARN_OK && b.final_pass > b.current_pass) {
	      b.final_pass = b.current_pass;
	   }

	   if (b.output_regularizer && b.final_pass == b.current_pass) {
	     zero_preconditioner(*all);
	     b.preconditioner_pass = true;
	   }

	   if(!all->holdout_set_off)
	   {
	     if(summarize_holdout_set(*all, b.no_win_counter))
               finalize_regressor(*all, all->final_regressor_name); 
	     if(b.early_stop_thres == b.no_win_counter)
	     { 
               all-> early_terminate = true;
               cerr<<"Early termination reached w.r.t. holdout set error";
             }

	   } 
           
       }else{//reaching convergence in the previous pass
        if(b.output_regularizer) 
           preconditioner_to_regularizer(*all, b, (*all).l2_lambda);
        b.current_pass ++;
      }   
                
  }
}

// placeholder
void predict(bfgs& b, learner& base, example& ec)
{
  vw* all = b.all;
  ec.final_prediction = bfgs_predict(*all,ec);
}

void learn(bfgs& b, learner& base, example& ec)
{
  vw* all = b.all;
  assert(ec.in_use);

  if (b.current_pass <= b.final_pass)
    {
      if(ec.test_only)
	{ 
	  label_data* ld = (label_data*)ec.ld;
	  predict(b, base, ec);
	  ec.loss = all->loss->getLoss(all->sd, ec.final_prediction, ld->label) * ld->weight;
	}
      else if (test_example(ec))
	predict(b, base, ec);
      else
	process_example(*all, b, ec);
    }
}

void finish(bfgs& b)
{
  b.predictions.delete_v();
  free(b.mem);
  free(b.rho);
  free(b.alpha);
}

void save_load_regularizer(vw& all, bfgs& b, io_buf& model_file, bool read, bool text)
{

  char buff[512];
  int c = 0;
  uint32_t stride = 1 << all.reg.stride_shift;
  uint32_t length = 2*(1 << all.num_bits);
  uint32_t i = 0;
  size_t brw = 1;
  do 
    {
      brw = 1;
      weight* v;
      if (read)
	{
	  c++;
	  brw = bin_read_fixed(model_file, (char*)&i, sizeof(i),"");
	  if (brw > 0)
	    {
	      assert (i< length);		
	      v = &(b.regularizers[i]);
	      if (brw > 0)
		brw += bin_read_fixed(model_file, (char*)v, sizeof(*v), "");
	    }
	}
      else // write binary or text
	{
	  v = &(b.regularizers[i]);
	  if (*v != 0.)
	    {
	      c++;
	      int text_len = sprintf(buff, "%d", i);
	      brw = bin_text_write_fixed(model_file,(char *)&i, sizeof (i),
					 buff, text_len, text);
	      
	      text_len = sprintf(buff, ":%f\n", *v);
	      brw+= bin_text_write_fixed(model_file,(char *)v, sizeof (*v),
					 buff, text_len, text);
	      if (read && i%2 == 1) // This is the prior mean
		all.reg.weight_vector[(i/2*stride)] = *v;
	    }
	}
      if (!read)
	i++;
    }  
  while ((!read && i < length) || (read && brw >0));
}


void save_load(bfgs& b, io_buf& model_file, bool read, bool text)
{
  vw* all = b.all;

  uint32_t length = 1 << all->num_bits;

  if (read)
    {
      initialize_regressor(*all);
      if (all->per_feature_regularizer_input != "")
	{
	  b.regularizers = (weight *)calloc_or_die(2*length, sizeof(weight));
	  if (b.regularizers == NULL)
	    {
	      cerr << all->program_name << ": Failed to allocate regularizers array: try decreasing -b <bits>" << endl;
	      throw exception();
	    }
	}
      int m = all->m;
      
      b.mem_stride = (m==0) ? CG_EXTRA : 2*m;
      b.mem = (float*) malloc(sizeof(float)*all->length()*(b.mem_stride));
      b.rho = (double*) malloc(sizeof(double)*m);
      b.alpha = (double*) malloc(sizeof(double)*m);
      
      if (!all->quiet) 
	{
	  fprintf(stderr, "m = %d\nAllocated %luM for weights and mem\n", m, (long unsigned int)all->length()*(sizeof(float)*(b.mem_stride)+(sizeof(weight) << all->reg.stride_shift)) >> 20);
	}
      
      b.net_time = 0.0;
      ftime(&b.t_start_global);
      
      if (!all->quiet)
	{
	  const char * header_fmt = "%2s %-10s\t%-10s\t%-10s\t %-10s\t%-10s\t%-10s\t%-10s\t%-10s\t%-10s\n";
	  fprintf(stderr, header_fmt,
		  "##", "avg. loss", "der. mag.", "d. m. cond.", "wolfe1", "wolfe2", "mix fraction", "curvature", "dir. magnitude", "step size");
	  cerr.precision(5);
	}
      
      if (b.regularizers != NULL)
	all->l2_lambda = 1; // To make sure we are adding the regularization
      b.output_regularizer =  (all->per_feature_regularizer_output != "" || all->per_feature_regularizer_text != "");
      reset_state(*all, b, false);
    }

  //bool reg_vector = b.output_regularizer || all->per_feature_regularizer_input.length() > 0;
  bool reg_vector = (b.output_regularizer && !read) || (all->per_feature_regularizer_input.length() > 0 && read);
    
  if (model_file.files.size() > 0)
    {
      char buff[512];
      uint32_t text_len = sprintf(buff, ":%d\n", reg_vector);
      bin_text_read_write_fixed(model_file,(char *)&reg_vector, sizeof (reg_vector),
				"", read,
				buff, text_len, text);
      
      if (reg_vector)
	save_load_regularizer(*all, b, model_file, read, text);
      else
	GD::save_load_regressor(*all, model_file, read, text);
    }
}

  void init_driver(bfgs& b)
  {
    b.backstep_on = true;
  }

learner* setup(vw& all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file)
{
  bfgs* b = (bfgs*)calloc_or_die(1,sizeof(bfgs));
  b->all = &all;
  b->wolfe1_bound = 0.01;
  b->first_hessian_on=true;
  b->first_pass = true;
  b->gradient_pass = true;
  b->preconditioner_pass = true;
  b->backstep_on = false;
  b->final_pass=all.numpasses;  
  b->no_win_counter = 0;
  b->early_stop_thres = 3;

  if(!all.holdout_set_off)
  {
    all.sd->holdout_best_loss = FLT_MAX;
    if(vm.count("early_terminate"))      
      b->early_stop_thres = vm["early_terminate"].as< size_t>();     
  }
  
  if (vm.count("hessian_on") || all.m==0) {
    all.hessian_on = true;
  }
  if (!all.quiet) {
    if (all.m>0)
      cerr << "enabling BFGS based optimization ";
    else
      cerr << "enabling conjugate gradient optimization via BFGS ";
    if (all.hessian_on)
      cerr << "with curvature calculation" << endl;
    else
      cerr << "**without** curvature calculation" << endl;
  }
  if (all.numpasses < 2)
    {
      cout << "you must make at least 2 passes to use BFGS" << endl;
      throw exception();
    }

  all.bfgs = true;
  all.reg.stride_shift = 2;

  learner* l = new learner(b, 1 << all.reg.stride_shift);
  l->set_learn<bfgs, learn>();
  l->set_predict<bfgs, predict>();
  l->set_save_load<bfgs,save_load>();
  l->set_init_driver<bfgs,init_driver>();
  l->set_end_pass<bfgs,end_pass>();
  l->set_finish<bfgs,finish>();

  return l;
}
}

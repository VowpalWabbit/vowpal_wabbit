/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */
#include <fstream>
#include <float.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <boost/math/special_functions/digamma.hpp>
#include "parse_example.h"
#include "constant.h"
#include "sparse_dense.h"
#include "gd.h"
#include "lda.h"
#include "cache.h"
#include "multisource.h"
#include "simple_label.h"
#include "delay_ring.h"

using namespace boost::math::policies;

size_t max_w = 0;

// // Ported from Tom Minka's lightspeed toolbox
// float mydigamma(float x) 
// {
//   double large = 9.5;
//   double d1 = -0.5772156649015328606065121;
//   double pi = 3.14159265358979323846264338327950288419716939937510;
//   double d2 = pi*pi/6.;
//   double small = 1e-6;
//   double s3 = 1./12.;
//   double s4 = 1./120.;
//   double s5 = 1./252.;
//   double s6 = 1./240.;
//   double s7 = 1./132.;
//   double s8 = 691./32760.;
//   double s9 = 1./12.;
//   double s10 = 3617./8160.;

//   double y = 0;

//   if (x <= 0) {
//     fprintf(stderr, "error --- passed nonpositive number to digamma()\n");
//     exit(1);
//   }
  
//   if (x <= small)
//     return d1 - 1 / x + d2 * x;

//   while(x < large) {
//     y = y - 1 / x;
//     x = x + 1;
//   }

//   double r = 1. / x;
//   y = y + log(x) - 0.5 * r;
//   r = r*r;
//   y = y - r * (s3 - r * (s4 - r * (s5 - r * (s6 - r * s7))));

//   return y;
// }

float mydigamma(float x) 
{
  return boost::math::digamma(x);
}

float decayfunc(float t, float old_t, float power_t) {
  float result = 1;
  for (float i = old_t+1; i <= t; i += 1)
    result *= (1-pow(i, -power_t));
  return result;
}

float decayfunc2(float t, float old_t, float power_t) 
{
  float power_t_plus_one = 1. - power_t;
  float arg =  - ( pow(t, power_t_plus_one) -
                   pow(old_t, power_t_plus_one));
  return exp ( arg
               / power_t_plus_one);
}

float decayfunc3(double t, double old_t, double power_t) 
{
  double power_t_plus_one = 1. - power_t;
  double logt = log(t);
  double logoldt = log(old_t);
  return (old_t / t) * exp(0.5*power_t_plus_one*(-logt*logt + logoldt*logoldt));
}

void expdigammify(float* gamma)
{
  float sum=0;
  for (size_t i = 0; i<global.lda; i++)
    {
      sum += gamma[i];
      gamma[i] = mydigamma(gamma[i]);
    }
  sum = mydigamma(sum);
  for (size_t i = 0; i<global.lda; i++)
    gamma[i] = exp(gamma[i] - sum);
}

void expdigammify_2(float* gamma, float* norm)
{
  for (size_t i = 0; i<global.lda; i++)
    {
      gamma[i] = exp(mydigamma(gamma[i]) - norm[i]);
    }
}

float average_diff(float* oldgamma, float* newgamma)
{
  float sum = 0.;
  for (size_t i = 0; i<global.lda; i++)
    sum += fabsf(oldgamma[i] - newgamma[i]);
  return sum;
}

float find_cw(float* u_for_w, float* v)
{
  float c_w = 0;
  for (size_t k =0; k<global.lda; k++)
    c_w += u_for_w[k]*v[k];

  return 1.f / c_w;
}

void start_lda(gd_thread_params t)
{
  regressor reg = t.reg;
  example* ec = NULL;

  double total_lambda[global.lda];
  for (size_t k = 0; k < global.lda; k++)
    total_lambda[k] = 0;
  size_t stride = global.stride;
  weight* weights = reg.weight_vectors[0];

  for (size_t i =0; i <= global.thread_mask;i+=stride)
    for (size_t k = 0; k < global.lda; k++)
      {
	total_lambda[k] += weights[i+k];
      }

  v_array<float> u_kw;
  while ( true )
    {//this is a poor man's select operation.
      if ((ec = get_example(0)) != NULL)//semiblocking operation.
	{
	  float new_gamma[global.lda];
	  float old_gamma[global.lda];

	  for (size_t i = 0; i < global.lda; i++)
	    {
	      new_gamma[i] = 1.;
	      old_gamma[i] = 0.;
	    }
	  size_t num_words =0;
	  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++)
	    {
	      num_words += ec->subsets[*i][1] - ec->subsets[*i][0];
	    }
	  u_kw.erase();
	  reserve(u_kw,num_words*global.lda);

	  float digammas[global.lda];
	  float additional = (float)(global.length()) * global.lda_rho;
	  for (size_t i = 0; i<global.lda; i++)
	    {
	      digammas[i] = mydigamma(total_lambda[i] + additional);
	    }
	  size_t word_count = 0;
	  float words_in_document = 0;
	  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++)
	    {
	      feature *f = ec->subsets[*i][0];

	      for (; f != ec->subsets[*i][1]; f++){
                words_in_document += f->x;
//                 fprintf(stderr, "f->weight_index & global.thread_mask = %d\n", f->weight_index & global.thread_mask);
		float* u_for_w = u_kw.begin+(global.lda*word_count);
		float* weights_for_w = &(weights[f->weight_index & global.thread_mask]);
//                 float olddecay = decayfunc(ec->example_t-1, weights_for_w[global.lda], t.vars->power_t);
//                 float testolddecay = decayfunc2(ec->example_t-1, weights_for_w[global.lda], t.vars->power_t);
//                 float testolddecay2 = decayfunc3(ec->example_t-1, weights_for_w[global.lda], t.vars->power_t);
//                 fprintf(stderr, "olddecay vs approximation = %e  vs  %e  vs %e\n", 1. - olddecay, 1. - testolddecay, 1. - testolddecay2);
                float olddecay = decayfunc3(ec->example_t-1, weights_for_w[global.lda], t.vars->power_t);
                float decay = decayfunc3(ec->example_t, weights_for_w[global.lda], t.vars->power_t);
//                 float decay = olddecay * (1-pow(ec->example_t, -t.vars->power_t)); //decayfunc(ec->example_t, weights_for_w[global.lda], t.vars->power_t);
// 		float decay = exp (
// 				   - ( pow(ec->example_t,power_t_plus_one) -
// 				       pow(weights_for_w[global.lda], power_t_plus_one))
// 				   / power_t_plus_one);
		weights_for_w[global.lda] = ec->example_t;
		for (size_t k = 0; k < global.lda; k++)
		  {
                    // we know we're changing this column of weights, so we
                    // subtract it out of total_lambda before it changes.
//                     fprintf(stderr, "total_lambda[%d] -= word_weights[%d] || %f -= %f || %f\n", k, k, total_lambda[k], weights_for_w[k], total_lambda[k]-weights_for_w[k]);
                    total_lambda[k] -= weights_for_w[k] * olddecay;
		    weights_for_w[k] *= decay;
		    u_for_w[k] = weights_for_w[k] + global.lda_rho;
		  }
		expdigammify_2(u_for_w, digammas);
		word_count++;
	      }
	    }
//           fprintf(stderr, "word_count = %d\n", word_count);

	  float v[global.lda];
          size_t numits = 0;


	  do
	    {
	      memcpy(v,new_gamma,sizeof(float)*global.lda);
	      expdigammify(v);

	      memcpy(old_gamma,new_gamma,sizeof(float)*global.lda);
	      for (size_t k = 0; k<global.lda; k++)
		new_gamma[k] = 0;

	      word_count = 0;
	      for (size_t* i = ec->indices.begin; i != ec->indices.end; i++)
		{
		  feature *f = ec->subsets[*i][0];
		  for (; f != ec->subsets[*i][1]; f++)
		    {
		      float* u_for_w = &u_kw[word_count*global.lda];
		      float c_w = find_cw(u_for_w,v);
                      float xc_w = c_w * f->x;
                      size_t max_k = global.lda;
		      for (size_t k =0; k<max_k; k++) {
			new_gamma[k] += xc_w*u_for_w[k];
                      }
		      word_count++;
		    }
		}
	      for (size_t k =0; k<global.lda; k++) {
		new_gamma[k] = new_gamma[k]*v[k]+global.lda_alpha;
              }
              numits++;
	    }
	  while (average_diff(old_gamma, new_gamma) > 0.1);

	  word_count=0;

	  float eta = global.eta/pow(ec->example_t,t.vars->power_t);
// 	  float minuseta = 1 - eta;
          float minuseta = decayfunc3(ec->example_t, ec->example_t-1, t.vars->power_t);
	  float delta[global.lda];
	  for (size_t k = 0; k < global.lda; k++)
	    delta[k] = 0.f;

	  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++)
	    {
	      feature *f = ec->subsets[*i][0];
	      for (; f != ec->subsets[*i][1]; f++)
		{
		  float* word_weights = &(weights[f->weight_index & global.thread_mask]);

		  float* u_for_w = &u_kw[word_count*global.lda];
		  float c_w = find_cw(u_for_w,v);

		  for (size_t k =0; k<global.lda;k++)
		    {
//                       fprintf(stderr, "total_lambda[%d] -= word_weights[%d] || %f -= %f || %f\n", k, k, total_lambda[k], word_weights[k], total_lambda[k]-word_weights[k]);
//                       total_lambda[k] -= word_weights[k];
//                       fprintf(stderr, "after subtracting off word_weights[%d], total_lambda[%d] = %f\n", k, k, total_lambda[k]);
		      float new_value = minuseta*word_weights[k]
			+ eta*(global.lda_D*f->x*u_for_w[k]*v[k]*c_w);
		      delta[k] += new_value;
		      word_weights[k] = new_value;
		    }
		}
	    }
	  for (size_t k = 0; k < global.lda; k++) {
//             fprintf(stderr, "total_lambda[%d] = %f. total_lambda[%d] *= %f || %f\n", k, total_lambda[k], k, minuseta, total_lambda[k]*minuseta);
            total_lambda[k] *= minuseta;
//             fprintf(stderr, "delta[%d] = %f\n", k, delta[k]);
	    total_lambda[k] += delta[k];
          }

//           double test_total[global.lda];
//           for (size_t k = 0; k < global.lda; k++)
//             test_total[k] = 0;
//           float power_t_plus_one = 1. - t.vars->power_t;
//           for (size_t i =0; i <= global.thread_mask;i+=stride) {
//             for (size_t k = 0; k < global.lda; k++)
//               {
//                 float decay = decayfunc3(ec->example_t, weights[i+global.lda], t.vars->power_t);
// //                 fprintf(stderr, "decay(%f, %f) = %f\n", ec->example_t, weights[i+global.lda], decay);
// // 		float decay = exp (
// // 				   - ( pow(ec->example_t-1,power_t_plus_one) -
// // 				       pow(weights[i+global.lda], power_t_plus_one))
// // 				   / power_t_plus_one);
//                 test_total[k] += weights[i+k]*decay;
//               }
//           }
//           for (size_t k = 0; k < global.lda; k++) {
//             fprintf(stderr, "diff between total_lambda[%d] and test_total[%d] = %f-%f = %f\n", k, k, total_lambda[k], test_total[k], total_lambda[k] - test_total[k]);
//           }

	  ec->topic_predictions.erase();
	  reserve(ec->topic_predictions,global.lda);
	  memcpy(ec->topic_predictions.begin,new_gamma,global.lda*sizeof(float));
	  if (global.audit)
	    print_audit_features(reg, ec);
	  finish_example(ec);
	}
      else if (thread_done(0))
	{
//           // print out all weights
	  for (size_t k = 0; k < global.lda; k++) {
	    for (size_t i = 0; i < global.length(); i++) {
	      fprintf(stdout, "%0.3f ", weights[i + k] + global.lda_rho);
	    }
	    fprintf(stdout, "\n");
	  }

	  if (global.local_prediction > 0)
	    shutdown(global.local_prediction, SHUT_WR);
	  return;
	}
      else
	;//busywait when we have predicted on all examples but not yet trained on all.
    }
}

void end_lda()
{
  
}

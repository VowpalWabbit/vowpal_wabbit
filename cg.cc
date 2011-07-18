/*
Copyright (c) 2009-2011 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license

The algorithm here is generally based on Jonathan Shewchuck's
tutorial.  
 */
#include <fstream>
#include <float.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "parse_example.h"
#include "constant.h"
#include "sparse_dense.h"
#include "cg.h"
#include "cache.h"
#include "multisource.h"
#include "simple_label.h"
#include "delay_ring.h"
#include "allreduce.h"
#include <sys/timeb.h>

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
  float update = g * page_feature.x;
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

// old first derivative is separate

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

double derivative_magnitude(regressor& reg, float* old_first_derivative)
{//compute derivative magnitude & shift new derivative to old
  double ret = 0.;
  uint32_t length = 1 << global.num_bits;
  size_t stride = global.stride;
  weight* weights = reg.weight_vectors[0];
  for(uint32_t i = 0; i < length; i++)
    {
      ret += weights[stride*i+1]*weights[stride*i+1]*weights[stride*i+3];
      old_first_derivative[i] = weights[stride*i+1];
      weights[stride*i+1] = 0;
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

double derivative_diff_mag(regressor& reg, float* old_first_derivative)
{//compute the derivative difference
  double ret = 0.;
  uint32_t length = 1 << global.num_bits;
  size_t stride = global.stride;
  weight* weights = reg.weight_vectors[0];
  for(uint32_t i = 0; i < length; i++)
    {
      ret += weights[stride*i+1]*weights[stride*i+3]*
	(weights[stride*i+1] - old_first_derivative[i]);
    }
  return ret;
}

double add_regularization(regressor& reg,float regularization)
{
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

void zero_state(regressor& reg, float* old_first_derivative)
{
  uint32_t length = 1 << global.num_bits;
  size_t stride = global.stride;
  weight* weights = reg.weight_vectors[0];
  for(uint32_t i = 0; i < length; i++) 
    {
      old_first_derivative[i] = 0;
      weights[stride*i+1] = 0;
      weights[stride*i+2] = 0;
      weights[stride*i+3] = 0;
    }
}

double derivative_in_direction(regressor& reg, float* old_first_derivative)
{
  double ret = 0.;
  uint32_t length = 1 << global.num_bits;
  size_t stride = global.stride;
  for(uint32_t i = 0; i < length; i++)
    ret += old_first_derivative[i]*reg.weight_vectors[0][stride*i+2];
  return ret;
}

void update_direction(regressor& reg, float old_portion, float* old_first_derivative)
{
  uint32_t length = 1 << global.num_bits;
  size_t stride = global.stride;
  weight* weights = reg.weight_vectors[0];
  
  for(uint32_t i = 0; i < length; i++)
    weights[stride*i+2] = old_first_derivative[i]*weights[stride*i+3] + old_portion * weights[stride*i+2];
}

void update_weight(regressor& reg, float step_size)
{
  uint32_t length = 1 << global.num_bits;
  size_t stride = global.stride;
  for(uint32_t i = 0; i < length; i++)
    reg.weight_vectors[0][stride*i] += step_size * reg.weight_vectors[0][stride*i+2];
}

void accumulate(node_socks socks, regressor& reg, size_t o) {
  ftime(&t_start);
  uint32_t length = 1 << global.num_bits; //This is size of gradient
  size_t stride = global.stride;
  float* local_grad = new float[length];
  weight* weights = reg.weight_vectors[0];
  for(uint32_t i = 0;i < length;i++) 
    {
      local_grad[i] = weights[stride*i+o];
    }

  all_reduce((char*)local_grad, length*sizeof(float), socks);
  for(uint32_t i = 0;i < length;i++) 
    {
      weights[stride*i+o] = local_grad[i];
    }
  delete[] local_grad;
  ftime(&t_end);
  net_comm_time += (int) (1000.0 * (t_end.time - t_start.time) + (t_end.millitm - t_start.millitm)); 
}

float accumulate_scalar(node_socks socks, float local_sum) {
  ftime(&t_start);
  float temp = local_sum;
  all_reduce((char*)&temp, sizeof(float), socks);
  ftime(&t_end);
  net_comm_time += (int) (1000.0 * (t_end.time - t_start.time) + (t_end.millitm - t_start.millitm)); 
  return temp;
}

void setup_cg(gd_thread_params& t)
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
 
  double previous_d_mag=0;
  size_t current_pass = 0;
  double previous_loss_sum = 0;

  float* old_first_derivative = (float*) malloc(sizeof(float)*global.length());

  node_socks socks;
  struct timeb t_start_global, t_end_global;
  double net_time = 0.0;
  double prev_comm_time = 0.0;
  net_comm_time = 0.0;
  ftime(&t_start_global);
  
  if(global.master_location != "")
    all_reduce_init(global.master_location, &socks);

  if (!global.quiet)
    {
      const char * header_fmt = "%-10s\t%-10s\t%-10s\t%-10s\t%-10s\t%-10s\n";
      fprintf(stderr, header_fmt,
	      "avg. loss", "mix fraction", "der. mag", "curvature", "step size", "newt. decr.");
      fflush(stderr);
      cerr.precision(5);
    }

  bool output_regularizer = false;
  if (global.per_feature_regularizer_output != "" || global.per_feature_regularizer_text != "") {
    if (reg.regularizers != NULL)
      global.regularization = 1; // To make sure we are adding the regularization
    output_regularizer = true;
  }

  while ( true )
    {
      if ((ec = get_example(thread_num)) != NULL)//semiblocking operation.
	{
	  assert(ec->in_use);
	  if (ec->pass != current_pass)//we need to do work on all features.
	    {
	      if (current_pass == 0)
		{
		  if(global.master_location != "")
		    {
		      accumulate(socks, reg, 3); //Accumulate preconditioner
		      importance_weight_sum = accumulate_scalar(socks, importance_weight_sum);
		    }
		  finalize_preconditioner(reg,global.regularization);
		}
	      if (gradient_pass) // We just finished computing all gradients
		{
		  if(global.master_location != "") {
		    loss_sum = accumulate_scalar(socks, loss_sum);  //Accumulate loss_sums
		    accumulate(socks, reg, 1); //Accumulate gradients from all nodes
		  }
		  if (global.regularization > 0.)
		    loss_sum += add_regularization(reg,global.regularization);
		  if (!global.quiet)
		    fprintf(stderr, "%-f\t", loss_sum / importance_weight_sum);
		  
		  if (current_pass > 0 && loss_sum > previous_loss_sum)
		    {// we stepped to far last time, step back
		      if (ec->pass != 0)
			step_size *= 0.5;//new data incoming, undo the step entirely.
		      if (!global.quiet)
			fprintf(stderr, "\t\t\t\tbackstep\t%e\n", -step_size);
		      update_weight(reg,- step_size);
		      zero_derivative(reg);
		      loss_sum = 0.;
		    }
		  else if (ec->pass != 0)
		    {
		      previous_loss_sum = loss_sum;
		      loss_sum = 0.;
		      example_number = 0;
		      curvature = 0;
		      float mix_frac = 0;
		      if (current_pass != 0)
			mix_frac = derivative_diff_mag(reg, old_first_derivative) / previous_d_mag;
		      if (mix_frac < 0 || isnan(mix_frac))
			mix_frac = 0;
		      float new_d_mag = derivative_magnitude(reg, old_first_derivative);
		      previous_d_mag = new_d_mag;
		      if (!global.quiet)
			fprintf(stderr, "%f\t%f\t", mix_frac, new_d_mag / importance_weight_sum);
		      
		      update_direction(reg, mix_frac, old_first_derivative);
		      gradient_pass = false;//now start computing curvature
		    }
		}
	      else // just finished all second gradients
		{
		  if(global.master_location != "") {
		    curvature = accumulate_scalar(socks, curvature);  //Accumulate curvatures
		  }
		  if (global.regularization > 0.)
		    curvature += regularizer_direction_magnitude(reg,global.regularization);
		  float dd = derivative_in_direction(reg, old_first_derivative);
		  if (curvature == 0. && dd != 0.)
		    {
		      cout << "your curvature is 0, something wrong.  Try adding regularization" << endl;
		      exit(1);
		    }
		  step_size = - dd/curvature;
		  if (!global.quiet) {
		    fprintf(stderr, "%-e\t%-e\t%-f\n", curvature/importance_weight_sum, step_size, 0.5*step_size*step_size*curvature /importance_weight_sum);
		    //fprintf(stdout, "Net comm. time is %f\n",net_comm_time - prev_comm_time);
		  }
		  prev_comm_time = net_comm_time;
		  predictions.erase();
		  update_weight(reg,step_size);

		  gradient_pass = true;
		}//now start computing derivatives.
	      if (ec->pass == 0)//new examples incoming, reset everything.
		{
		  zero_state(reg,old_first_derivative);//set all except weights to 0.
		  predictions.erase();

		  example_number=0;
		  curvature=0.;
		  gradient_pass=true;
		  loss_sum = 0;
		  step_size = 0.;
		  importance_weight_sum = 0.;
 
		  previous_d_mag=0;
		  current_pass = 0;
		  previous_loss_sum = 0;
		}
	      else
		current_pass++;
	      if (output_regularizer && current_pass == global.numpasses - 1)
		zero_preconditioner(reg);
	    }
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
	      label_data* ld = (label_data*)ec->ld;
	      importance_weight_sum += ld->weight;
	      update_preconditioner(reg,ec);//w[3]
	    }
	  finish_example(ec);
	}
      else if (thread_done(thread_num))
	{
	  if (example_number == predictions.index())//do one last update
	    {
	      if(global.master_location != "") {
		curvature = accumulate_scalar(socks, curvature);  //Accumulate curvatures
	      }
	      if (global.regularization > 0.)
		curvature += regularizer_direction_magnitude(reg,global.regularization);
	      float dd = derivative_in_direction(reg, old_first_derivative);
	      if (curvature == 0. && dd != 0.)
		{
		  cout << "your curvature is 0, something wrong.  Try adding regularization" << endl;
		  exit(1);
		}
	      float step_size = - dd/(max(curvature,1.));
	      if (!global.quiet) {
		fprintf(stderr, "%-e\t%-e\t%-f\n", curvature, step_size, 0.5*step_size*step_size*curvature/importance_weight_sum);
		//fprintf(stdout, "Net comm. time is %f\n",net_comm_time - prev_comm_time);
	      }
	      update_weight(reg,step_size);
	    }
	  if (output_regularizer)//need to accumulate and place the regularizer.
	    {
	      if(global.master_location != "")
		accumulate(socks, reg, 3); //Accumulate preconditioner
	      preconditioner_to_regularizer(reg,global.regularization);
	    }
	  ftime(&t_end_global);
	  net_time += (int) (1000.0 * (t_end_global.time - t_start_global.time) + (t_end_global.millitm - t_start_global.millitm)); 
	  if (!global.quiet)
	    {
	      cerr<<"Net time spent in communication = "<<(float)net_comm_time/(float)1000<<" seconds\n";
	      cerr<<"Net time spent = "<<(float)net_time/(float)1000<<" seconds\n";
	    }
	  if (global.local_prediction > 0)
	    shutdown(global.local_prediction, SHUT_WR);
	  if(global.master_location != "")
	    all_reduce_close(socks);
	  free(predictions.begin);
	  free(old_first_derivative);
	  free(ec);
	  t.reg = reg;
	  return;
	}
      else 
	;//busywait when we have predicted on all examples but not yet trained on all.
    }
  
  if(global.master_location != "")
    all_reduce_close(socks);
  free(predictions.begin);
  free(old_first_derivative);
  ftime(&t_end_global);
  net_time += (int) (1000.0 * (t_end_global.time - t_start_global.time) + (t_end_global.millitm - t_start_global.millitm)); 
  cerr<<"Net time spent in communication = "<<(float)net_comm_time/(float)1000<<"seconds\n";
  cerr<<"Net time spent = "<<(float)net_time/(float)1000<<"seconds\n";
  fflush(stderr);

  t.reg = reg;
  return;
}

void destroy_cg()
{
}


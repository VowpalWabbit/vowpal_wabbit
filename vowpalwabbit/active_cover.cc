#include <errno.h>
#include "gd.h"
#include "reductions.h"
#include "rand48.h"
#include "float.h"
#include "vw.h"
#include "active_cover.h"

using namespace LEARNER;

inline float sign(float w){ if (w < 0.) return -1.; else  return 1.;}
inline float max(float a, float b){if(a > b) return a; else return b;}
inline float min(float a, float b){if(a < b) return a; else return b;}

bool dis_test(vw& all, example& ec, float prediction, float threshold)
{
	// Get loss difference
	float k = ec.example_t - ec.l.simple.weight;
	ec.revert_weight = all.loss->getRevertingWeight(all.sd, prediction, all.eta/powf(k,all.power_t));
	float loss_delta = ec.revert_weight/k;

	bool result = (loss_delta <= threshold);
	return result;	
}
  
float get_threshold(float sum_loss, float t, float c0, float alpha)
{
	if(t < 3.)
	{
		return 1.0;
	}
	else
	{
		float avg_loss = sum_loss/t;
		float threshold = sqrt(c0*avg_loss/t) + max(2*alpha,4.0)*c0*log(t)/t;
		return threshold;
	}
}

float get_pmin(float sum_loss, float t)
{
	float avg_loss = sum_loss/t;
	float pmin = min(1.0/(sqrt(t*avg_loss)+log(t)),0.5);
	return pmin; // treating n*eps_n = 1
}

float query_decision(active_cover& a, base_learner& l, example& ec, float prediction, float pmin)
{
	size_t cover_size = a.weights->get_size();	
	float q2 = 4*pmin*pmin;
	float lambda, max_lambda = -1;
	size_t n_dis = 0;

	for(size_t i = 0; i < cover_size; i++)
	{
		l.predict(ec,i+1);
		lambda = a.weights->get(LAMBDA, i);
		q2 += (sign(ec.pred.scalar) != sign(prediction)) ? lambda : 0;
		n_dis += (sign(ec.pred.scalar) != sign(prediction)) ? 1 : 0;
		max_lambda = max(lambda, max_lambda); 
	}

	float p = sqrt(q2)/(1+sqrt(q2));		

	if(nanpattern(p))
	{
		std::cout << "p: " << p << ". Set to 1. Max lambda = "<<  max_lambda <<endl;
		p = 1;
	}

	if(frand48() <= p)
	{
		return 1.f/p;
	}
	else
	{
		return -1.;
	}
}


float get_cost(float target_label, float ec_true_label, float prediction, bool in_dis, float s,  float threshold, float beta, float importance, float t)
{
	float c = 2*threshold*beta*beta*t;
	float cost = 0.0;

	if(in_dis)
	{
		bool add_s = (s>0 && sign(target_label) != sign(prediction)) || (s<0 && sign(target_label) == sign(prediction));
		cost += (add_s) ? fabs(s) : 0;
		cost += (sign(target_label) != sign(ec_true_label)) ? max(importance,0)*c : 0; 	
	}
	else
	{
		cost += (sign(target_label) != sign(prediction)) ? c : 0;
	}

	return cost;
}


template <bool is_learn>
void predict_or_learn_active_cover(active_cover& a, base_learner& base, example& ec) 
{
 	base.predict(ec, 0);
    
	if (is_learn)
	{
		vw& all = *a.all;
		float prediction = ec.pred.scalar;
		float t = ec.example_t - ec.l.simple.weight;
		float ec_input_weight = ec.l.simple.weight; 
		float ec_input_label = ec.l.simple.label;


		// Compute threshold defining allowed set A
		float threshold = get_threshold(all.sd->sum_loss, t, a.active_c0, a.alpha);
	
		// Make query decision	
		float importance = 1; 
		float pmin = 1;
		bool in_dis = true; 

		if(ec.example_t > 3)
		{
			// Test if ec is in DIS(A)
			in_dis = dis_test(all, ec, prediction, threshold);
			pmin = get_pmin(all.sd->sum_loss, t); 
			if(in_dis)
			{
				importance = (a.oracular) ? 1 : query_decision(a, base, ec, prediction, pmin);
			}
		}

		all.sd->n_in_dis += (in_dis) ? 1 : 0;
	
		// Query (or not)
		if(!in_dis) // Use predicted label 
		{
			ec.l.simple.label = sign(prediction);
			ec.l.simple.weight = ec_input_weight;
			base.learn(ec, 0);
			
		}
		else if(importance > 0) // Use importance-weighted example
		{
			all.sd->queries += 1;
			ec.l.simple.weight = ec_input_weight * importance;
			ec.l.simple.label = ec_input_label;
			base.learn(ec, 0);
		}
		else // skipped example
		{
			// Make sure the loss computation does not include
			// skipped examples
			ec.l.simple.label = FLT_MAX;
		}


		// Update the cost-sensitive learners in the cover and their weights

		float q2 = 4*pmin*pmin; 
		float beta = sqrt(a.alpha/a.active_c0)/10.0;
		float p, s, cost, cost_delta, num_delta, den_delta, lambda_delta;
		float num, den;
		float ec_output_label = ec.l.simple.label;
		float ec_output_weight = ec.l.simple.weight;	

		size_t cover_size = a.weights->get_size();	
		for(size_t i = 0; i < cover_size; i++)
		{
			// Incorporating updates to previous cost-sensitive learners
			q2 += (i > 0 && sign(ec.pred.scalar) != sign(prediction)) ? a.weights->get(LAMBDA,i-1) : 0;
			p = sqrt(q2)/(1 + sqrt(q2));
			s = 2*a.alpha*a.alpha - 1/p;

			// Set up costs	
			cost = get_cost(sign(prediction), ec_input_label, prediction, in_dis, s, threshold, beta, importance, t);
			cost_delta = cost - get_cost(-1.0*sign(prediction), ec_input_label, prediction, in_dis, s, threshold, beta, importance, t);

			// Choose min-cost label as the label
			ec.l.simple.label = sign(prediction);
			if(cost_delta > 0)
			{
				ec.l.simple.label = -1.0*sign(prediction);
			}

			// Set importance weight to be the cost difference
			ec.l.simple.weight = ec_input_weight * max(fabs(cost_delta),0.00001);

			// Update learner and weight
			base.learn(ec,i+1);
			base.predict(ec,i+1);

			// Update numerator of lambda			
			num_delta = (sign(ec.pred.scalar) == sign(prediction)) ? cost : (cost - cost_delta);
			num_delta += (in_dis) ? min(s,0) : 0;
			num_delta *= -2;			
			num = a.weights->update(LAMBDA_NUM, i, num_delta);
	
			if(num < 0)
			{
				//std::cout << "Warning: numerator of lambda " << (i+1) << " becomes negative. Reset to 0." << endl;
				a.weights->reset(LAMBDA_NUM, i);
				num = a.weights->get(LAMBDA_NUM, i);
			}

			// Update denominator of lambda
			den_delta = (sign(ec.pred.scalar) != sign(prediction) && in_dis) ? 1.0/pow(q2,1.5) : 0;
			den = a.weights->update(LAMBDA_DEN, i, den_delta);

			// Update lambda
			lambda_delta = num/den;
			if(nanpattern(lambda_delta))
			{
				//std::cout << "Warning: delta of lambda " << (i+1) << " is NaN. Reset to 0." << endl;
				lambda_delta = 0;
			}
	
			a.weights->update(LAMBDA, i, lambda_delta);
				
		}

		// Restoring the weight, the label, and the prediction
		ec.l.simple.weight = ec_output_weight;
		ec.l.simple.label = ec_output_label;
		ec.pred.scalar = prediction;
	}
}

base_learner* active_cover_setup(vw& all)
{
	//parse and set arguments
	if(missing_option(all, false, "active_cover", "enable active learning with cover")) 
	{
		return nullptr;
	}
	
	new_options(all, "Active Learning with cover options")
    	("mellowness", po::value<float>(), "active learning mellowness parameter c_0. Default 8.")
    	("alpha", po::value<float>(), "active learning variance upper bound parameter alpha. Default 1.")
    	("cover", po::value<float>(), "cover size. Default 20.")
    	("oracular", "Use Oracular-CAL style query or not. Default false.");
	add_options(all);


	active_cover& data = calloc_or_die<active_cover>();
	data.active_c0 = 1;
	data.alpha = 1;
	data.all = &all;
	data.oracular = false;

 	size_t cover_size = 20;
	if(all.vm.count("mellowness"))
	{
		data.active_c0 = all.vm["mellowness"].as<float>();
	}
  
	if(all.vm.count("alpha"))
	{
		data.alpha = all.vm["alpha"].as<float>();
	}

	if(all.vm.count("cover"))
	{
		cover_size = all.vm["cover"].as<float>();
	}

	if(all.vm.count("oracular"))
	{
		data.oracular = true;
		cover_size = 0;
	}
	
	*all.file_options <<" --active_cover --cover "<< cover_size;
  	base_learner* base = setup_base(all);
 
	data.weights = new active_cover_weights(cover_size);
  	//Create new learner
  	learner<active_cover>* l;
	l = &init_learner(&data, base, predict_or_learn_active_cover<true>, predict_or_learn_active_cover<false>, cover_size + 1);
  
	return make_base(*l);
}

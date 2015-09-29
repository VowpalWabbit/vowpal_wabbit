#include <errno.h>
#include "reductions.h"
#include "rand48.h"
#include "float.h"
#include "vw.h"
#include "active_cover.h"

using namespace LEARNER;

inline float sign(float w){ if (w < 0.f) return -1.f; else  return 1.f;}

enum weight_type_t 
{
	LAMBDA_NUM,
	LAMBDA_DEN,
	LAMBDA
};

const static size_t N_WEIGHT_TYPES = 3;

class active_cover_weights 
{	

public:
	active_cover_weights(size_t s) : size(s)
	{

		weights = v_init<float>();
		weights.resize(N_WEIGHT_TYPES*s);
		for(size_t i=0; i<(N_WEIGHT_TYPES*s); i++)
		{
			weights.push_back(0);
		} 
	}

	~active_cover_weights();

	float update(weight_type_t type, size_t i, float delta)
	{
		weights[type*size+i] += delta;
		return weights[type*size+i];
	}
	
	float set(weight_type_t type, size_t i, float new_value)
	{
		weights[type*size+i] = new_value;
		return weights[type*size+i];
	}

	float get(weight_type_t type, size_t i)
	{
		return weights[type*size+i];
	}
	
	size_t get_size()
	{
		return size;
	}

	void reset(weight_type_t type, size_t i)
	{
		weights[type*size+i] = 0;
	}

	void reset()
	{
		for(size_t i=0; i<weights.size(); i++)
		{
			weights[i] = 0;
		}
	}

private:	
	size_t size;
	v_array<float> weights;
};

struct active_cover
{
	// active learning algorithm parameters
	float active_c0;
	float alpha;
	float beta_scale;
	bool oracular;

	active_cover_weights* weights;
	vw* all;//statistics, loss
	LEARNER::base_learner* l; 
};

bool dis_test(vw& all, example& ec, float prediction, float threshold)
{
	if(ec.example_t <= 3)
	{
		return true;
	}

	// Get loss difference
	float k = ec.example_t - ec.l.simple.weight;
	ec.revert_weight = all.loss->getRevertingWeight(all.sd, prediction, all.eta/powf(k,all.power_t));
	float loss_delta = ec.revert_weight/k;

	bool result = (loss_delta <= threshold);

	return result;	
}
  
float get_threshold(float sum_loss, float t, float c0, float alpha)
{
	if(t < 3.f)
	{
		return 1.f;
	}
	else
	{
		float avg_loss = sum_loss/t;
		float threshold = sqrt(c0*avg_loss/t) + fmax(2.f*alpha,4.f)*c0*log(t)/t;
		return threshold;
	}
}

float get_pmin(float sum_loss, float t)
{
	// t = ec.example_t - 1
	if(t<=2.f)
	{
		return 1.f;
	}

	float avg_loss = sum_loss/t;
	float pmin = fmin(1.f/(sqrt(t*avg_loss)+log(t)),0.5);
	return pmin; // treating n*eps_n = 1
}

float query_decision(active_cover& a, base_learner& l, example& ec, float prediction, float pmin, bool in_dis)
{
	
	if(ec.example_t <= 3)
	{
		return 1.f;
	}
	
	if(!in_dis)
	{
		return -1.f;
	}

	if(a.oracular)
	{
		return 1.f;
	}

	size_t cover_size = a.weights->get_size();	
	float p, q2 = 4.f*pmin*pmin;

	for(size_t i = 0; i < cover_size; i++)
	{
		l.predict(ec,i+1);
		q2 += ((float)(sign(ec.pred.scalar) != sign(prediction)))*a.weights->get(LAMBDA,i);
	}

	p = sqrt(q2)/(1+sqrt(q2));		

	if(nanpattern(p))
	{
		p = 1.f;
	}

	if(frand48() <= p)
	{
		return 1.f/p;
	}
	else
	{
		return -1.f;
	}
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
		float threshold = get_threshold((float)all.sd->sum_loss, t, a.active_c0, a.alpha);
		float in_dis =  dis_test(all, ec, prediction, threshold);
		float pmin = get_pmin((float)all.sd->sum_loss, t); 
		float importance = query_decision(a, base, ec, prediction, pmin, in_dis);

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

		// Update the learners in the cover and their weights
		float q2 = 4.f*pmin*pmin; 
		float beta = (float)(sqrt(a.alpha/a.active_c0)/a.beta_scale);
		float p, s, cost, cost_delta, num_delta, den_delta, lambda;
		float num, den;
		float ec_output_label = ec.l.simple.label;
		float ec_output_weight = ec.l.simple.weight;
		float r = 2.f*threshold*beta*beta*t;
			
		// Set up costs	
		//cost = cost of predicting erm's prediction
		//cost_delta = cost - cost of predicting the opposite label
		if(in_dis)
		{
			cost = r*(fmax(importance,0.f))*((float)(sign(prediction) != sign(ec_input_label)));
		}
		else
		{
			cost = 0.f;
			cost_delta = -r;
		}

		size_t cover_size = a.weights->get_size();
		for(size_t i = 0; i < cover_size; i++)
		{
			// Incorporating weights of previous learners in the cover
			q2 += ((float)(i > 0 && sign(ec.pred.scalar) != sign(prediction))) * a.weights->get(LAMBDA,i-1);
			
			// Update cost
			if(in_dis)
			{
				p = sqrt(q2)/(1.f + sqrt(q2));
				s = 2.f*a.alpha*a.alpha - 1.f/p;
				cost_delta = 2.f*cost - r*(fmax(importance,0.f)) - s;
			}

			// Choose min-cost label as the label
			// Set importance weight to be the cost difference
			ec.l.simple.label = -1.f*sign(cost_delta)*sign(prediction);
			ec.l.simple.weight = ec_input_weight * fmax(fabs(cost_delta),0.00001);

			// Update learner and weight
			base.learn(ec,i+1);
			base.predict(ec,i+1);

			// Update numerator of lambda			
			num_delta = 2.f*((float)(sign(ec.pred.scalar) != sign(prediction))) * cost_delta;	
			num = a.weights->update(LAMBDA_NUM, i, num_delta);
			
			if(num < 0.f)
			{
				a.weights->reset(LAMBDA_NUM, i);
				num = a.weights->get(LAMBDA_NUM, i);
			}

			// Update denominator of lambda
			den_delta = ((float)(sign(ec.pred.scalar) != sign(prediction) && in_dis)) / pow(q2,1.5);
			den = a.weights->update(LAMBDA_DEN, i, den_delta);

			// Update lambda
			lambda = num/den;
			if(nanpattern(lambda))
			{
				lambda = 0.f;
			}

			a.weights->set(LAMBDA, i, lambda);
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
    	("beta_scale", po::value<float>(), "active learning variance upper bound parameter beta_scale. Default sqrt(10).")
    	("cover", po::value<float>(), "cover size. Default 12.")
    	("oracular", "Use Oracular-CAL style query or not. Default false.");
	add_options(all);

	active_cover& data = calloc_or_die<active_cover>();
	data.active_c0 = 8.f;
	data.alpha = 1.f;
	data.beta_scale = sqrt(10.f);
	data.all = &all;
	data.oracular = false;

 	size_t cover_size = 12;
	if(all.vm.count("mellowness"))
	{
		data.active_c0 = all.vm["mellowness"].as<float>();
	}
  
	if(all.vm.count("alpha"))
	{
		data.alpha = all.vm["alpha"].as<float>();
	}
	
	if(all.vm.count("beta_scale"))
	{
		data.beta_scale = all.vm["beta_scale"].as<float>();
	}

	if(all.vm.count("cover"))
	{
		cover_size = (size_t)all.vm["cover"].as<float>();
	}

	if(all.vm.count("oracular"))
	{
		data.oracular = true;
		cover_size = 0;
	}
  
	if (count(all.args.begin(), all.args.end(),"--lda") != 0)
	{
	    THROW("error: you can't combine lda and active learning");
	}


  	if (count(all.args.begin(), all.args.end(),"--active ") != 0)
	{
	    THROW("error: you can't use --active_cover and --active at the same time");
	}

	*all.file_options <<" --active_cover --cover "<< cover_size;
  	base_learner* base = setup_base(all);
 
	data.weights = new active_cover_weights(cover_size);

	for(size_t i = 0; i < cover_size; i++)
	{
		data.weights->set(LAMBDA_DEN, i, 1.0/8.0);
	}
  	//Create new learner
  	learner<active_cover>* l;
	l = &init_learner(&data, base, predict_or_learn_active_cover<true>, predict_or_learn_active_cover<false>, cover_size + 1);
  
	return make_base(*l);
}

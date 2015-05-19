#pragma once


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

	virtual ~active_cover_weights()
	{ }

	/* 
	float update_lambda(size_t i, float delta)
	{
		return (lambda[i] += delta);
	}
	
	float update_lambda_num(size_t i, float delta)
	{
		return (lambda_num[i] += delta);
	}
	
	float update_lambda_den(size_t i, float delta)
	{
		return (lambda_den[i] += delta);
	}
	
	float get_lambda(size_t i)
	{
		return lambda[i];
	}	
	*/


	float update(weight_type_t type, size_t i, float delta)
	{
		weights[type*size+i] += delta;
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
	bool no_beta;
	bool print_used;
	bool oracular;

	//uint32_t k;  
	
	active_cover_weights* weights;
	vw* all;//statistics, loss
	LEARNER::base_learner* l; 
	//COST_SENSITIVE::label cs_label;
};

LEARNER::base_learner* active_cover_setup(vw& all);

#include <float.h>
#include "reductions.h"
#include "cb_algs.h"
#include "rand48.h"
#include "bs.h"
#include "../explore/cpp/MWTExplorer.h"
#include "vw.h"

//In the future, the above two's names should be changed to
//WARM_START and INTERACTION
#define SUPERVISED 1
#define BANDIT 2

#define SUPERVISED_WS 1
#define BANDIT_WS 2

#define UAR 1
#define CIRCULAR 2
#define OVERWRITE 3

#define BANDIT_VALI 1
#define SUPERVISED_VALI 2

#define INSTANCE_WT 1
#define DATASET_WT 2

#define ABS_CENTRAL 1
#define MINIMAX_CENTRAL 2
#define MINIMAX_CENTRAL_ZEROONE 3


using namespace LEARNER;
using namespace MultiWorldTesting;
using namespace MultiWorldTesting::SingleAction;
using namespace ACTION_SCORE;

struct cbify;

//Scorer class for use by the exploration library
class vw_scorer : public IScorer<example>
{
public:
  vector<float> Score_Actions(example& ctx);
};

struct vw_recorder : public IRecorder<example>
{
  void Record(example& context, u32 a, float p, string /*unique_key*/)
  { }

  virtual ~vw_recorder()
  { }
};

struct cbify_adf_data
{
  example* ecs;
  example* empty_example;
  size_t num_actions;
};

struct cbify
{
  CB::label cb_label;
	COST_SENSITIVE::label cs_label;
  GenericExplorer<example>* generic_explorer;
  //v_array<float> probs;
  vw_scorer* scorer;
  MwtExplorer<example>* mwt_explorer;
  vw_recorder* recorder;
  v_array<action_score> a_s;
  // used as the seed
  size_t example_counter;
  vw* all;
  bool use_adf; // if true, reduce to cb_explore_adf instead of cb_explore
  cbify_adf_data adf_data;
  float loss0;
  float loss1;

	size_t choices_lambda;
	size_t warm_start_period;
	size_t bandit_period;

	v_array<float> cumulative_costs;
	v_array<float> lambdas;
	size_t num_actions;
	bool ind_bandit;
	bool ind_supervised;
	COST_SENSITIVE::label* csls;
	COST_SENSITIVE::label* csl_empty;
	CB::label* cbls;
	CB::label* cbl_empty;
	polyprediction pred;


	bool warm_start;
	float* old_weights;

	float corrupt_prob_supervised;
	float corrupt_prob_bandit;
	size_t corrupt_type_supervised;
	size_t corrupt_type_bandit;
	size_t corrupted_label;

	size_t validation_method;
	size_t bandit_iter;
	size_t warm_start_iter;
	size_t weighting_scheme;
	v_array<example> supervised_validation;
	size_t lambda_scheme;
	float epsilon;
	float cumulative_variance;
	size_t overwrite_label;
	size_t warm_start_type;
	size_t mc_pred;

};

float minimax_lambda(float epsilon, size_t num_actions, size_t warm_start_period, size_t bandit_period, size_t dim)
{
	/*
	if ( (epsilon / num_actions) * bandit_period >= dim )
		return 1.0;
	else
	{
		float z = sqrt( dim * ( (epsilon / num_actions) * bandit_period + warm_start_period) - (epsilon / num_actions) * bandit_period * warm_start_period );

		float numer = (epsilon / num_actions) + warm_start_period * (epsilon / num_actions) * (1/z);
		float denom = 1 + (epsilon / num_actions) + (warm_start_period - bandit_period) * (epsilon / num_actions) * (1/z);

		//cout<<"z = "<<z<<endl;
		//cout<<"numer = "<<numer<<endl;
		//cout<<"denom = "<<denom<<endl;
		return numer / denom;

	}
	*/
	return epsilon / (num_actions + epsilon);
}

void setup_lambdas(cbify& data, example& ec)
{
	// The lambdas are in fact arranged in ascending order (the middle lambda is 0.5)
	v_array<float>& lambdas = data.lambdas;

	//bandit only
	if (!data.ind_supervised && data.ind_bandit)
	{
		for (uint32_t i = 0; i<data.choices_lambda; i++)
			lambdas[i] = 1.0;
		return;
	}

	//supervised only
	if (!data.ind_bandit && data.ind_supervised)
	{
		for (uint32_t i = 0; i<data.choices_lambda; i++)
			lambdas[i] = 0.0;
		return;
	}

	//if no supervised and no bandit, then as there are no updates anyway,
	//we are still fine

	uint32_t mid = data.choices_lambda / 2;

	if (data.lambda_scheme == ABS_CENTRAL)
		lambdas[mid] = 0.5;
	else
		lambdas[mid] = minimax_lambda(data.epsilon, data.num_actions, data.warm_start_period, data.bandit_period, ec.num_features);

	for (uint32_t i = mid; i > 0; i--)
		lambdas[i-1] = lambdas[i] / 2;

	for (uint32_t i = mid+1; i < data.choices_lambda; i++)
		lambdas[i] = 1 - (1-lambdas[i-1]) / 2;

	if (data.lambda_scheme == MINIMAX_CENTRAL_ZEROONE)
	{
		lambdas[0] = 0.0;
		lambdas[data.choices_lambda-1] = 1.0;
	}

	//cout<<"lambdas:"<<endl;
	//for (uint32_t i = 0; i < data.choices_lambda; i++)
	//	cout<<lambdas[i]<<endl;

}


float rand_zeroone(vw* all)
{
	float f = merand48(all->random_state);
	//cout<<f<<endl;
	return f;
}


size_t generate_uar_action(cbify& data)
{
	float rand = rand_zeroone(data.all);
	//cout<<rand<<endl;

	for (size_t i = 1; i <= data.num_actions; i++)
	{
		if (rand <= float(i) / data.num_actions)
			return i;
	}
	return data.num_actions;

}

size_t corrupt_action(size_t action, cbify& data, size_t ec_type)
{
	float corrupt_prob;
	size_t corrupt_type;

	if (ec_type == SUPERVISED)
	{
		corrupt_prob = data.corrupt_prob_supervised;
		corrupt_type = data.corrupt_type_supervised;
	}
	else
	{
		corrupt_prob = data.corrupt_prob_bandit;
		corrupt_type = data.corrupt_type_bandit;
	}

	float rand = rand_zeroone(data.all);
	if (rand < corrupt_prob)
	{
		if (corrupt_type == UAR)
			return generate_uar_action(data);
		else if (corrupt_type == OVERWRITE)
			return data.overwrite_label;
		else
			return (action % data.num_actions) + 1;
	}
	else
		return action;

}

vector<float> vw_scorer::Score_Actions(example& ctx)
{
  vector<float> probs_vec;
  for(uint32_t i = 0; i < ctx.pred.a_s.size(); i++)
    probs_vec.push_back(ctx.pred.a_s[i].score);
  return probs_vec;
}

float loss(cbify& data, uint32_t label, uint32_t final_prediction)
{
  if (label != final_prediction)
    return data.loss1;
  else
    return data.loss0;
}

template<class T> inline void delete_it(T* p) { if (p != nullptr) delete p; }

bool ind_update(cbify& data, size_t ec_type)
{
	if (ec_type == SUPERVISED)
		return data.ind_supervised;
	else
		return data.ind_bandit;
}

float compute_weight_multiplier(cbify& data, size_t i, size_t ec_type)
{
	float weight_multiplier;

	if (ec_type == SUPERVISED)
	{
		if (data.lambdas[i] >= 0.5)
		 	weight_multiplier = (1 - data.lambdas[i]) / data.lambdas[i];
		else
			weight_multiplier = 1;
	}
	else
	{
		if (data.lambdas[i] >= 0.5)
			weight_multiplier = 1;
		else
			weight_multiplier = data.lambdas[i] / (1-data.lambdas[i]);

		if (data.weighting_scheme == DATASET_WT)
			weight_multiplier = weight_multiplier * data.warm_start_period / ( (data.bandit_iter+1) * (data.bandit_iter+2) );
	}
	return weight_multiplier;
}

uint32_t find_min(v_array<float> arr)
{
	float min_val = FLT_MAX;
	uint32_t argmin = 0;

	for (uint32_t i = 0; i < arr.size(); i++)
	{
		//cout<<arr[i]<<endl;
		if (arr[i] < min_val)
		{
			min_val = arr[i];
			argmin = i;
		}
	}

	return argmin;
}


void finish(cbify& data)
{
  //CB::cb_label.delete_label(&data.cb_label);
  //data.probs.delete_v();
  delete_it(data.scorer);
  delete_it(data.generic_explorer);
  delete_it(data.mwt_explorer);
  delete_it(data.recorder);
  data.a_s.delete_v();
	data.lambdas.delete_v();
	data.cumulative_costs.delete_v();

	if (data.validation_method == SUPERVISED_VALI)
	{
		for (size_t i = 0; i < data.warm_start_period; ++i)
			VW::dealloc_example(MULTICLASS::mc_label.delete_label, data.supervised_validation[i]);
		data.supervised_validation.delete_v();
	}

  if (data.use_adf)
  {
	  for (size_t a = 0; a < data.adf_data.num_actions; ++a)
			VW::dealloc_example(CB::cb_label.delete_label, data.adf_data.ecs[a]);

	  VW::dealloc_example(CB::cb_label.delete_label, *data.adf_data.empty_example);

    free(data.adf_data.ecs);
    free(data.adf_data.empty_example);

		//TODO: Use CB::cb_label.delete_label / CS here
		for (size_t a = 0; a < data.adf_data.num_actions; ++a)
			data.csls[a].costs.delete_v();

		data.csl_empty->costs.delete_v();

    free(data.csls);
		free(data.csl_empty);

    free(data.cbls);
    free(data.cbl_empty);

    free(data.old_weights);
  }
	else
	{
		COST_SENSITIVE::cs_label.delete_label(&data.cs_label);
		CB::cb_label.delete_label(&data.cb_label);
	}

}

void copy_example_to_adf(cbify& data, example& ec)
{
  auto& adf_data = data.adf_data;
  const uint64_t ss = data.all->weights.stride_shift();
  const uint64_t mask = data.all->weights.mask();

  for (size_t a = 0; a < adf_data.num_actions; ++a)
  {
    auto& eca = adf_data.ecs[a];
    // clear label
    auto& lab = eca.l.cb;
    CB::cb_label.default_label(&lab);

    // copy data
    VW::copy_example_data(false, &eca, &ec);

    // offset indicies for given action
    for (features& fs : eca)
    {
      for (feature_index& idx : fs.indicies)
      {
        idx = ((((idx >> ss) * 28904713) + 4832917 * (uint64_t)a) << ss) & mask;
      }
    }

    // avoid empty example by adding a tag (hacky)
    if (CB_ALGS::example_is_newline_not_header(eca) && CB::example_is_test(eca))
    {
      eca.tag.push_back('n');
    }
  }
}

void convert_mc_to_cs(cbify& data, example& ec)
{
	//generate cost-sensitive label (only for CSOAA's use - this will be retracted at the end)
	COST_SENSITIVE::label& csl = data.cs_label;
	size_t label = ec.l.multi.label;

	for (uint32_t j = 0; j < data.num_actions; j++)
	{
		csl.costs[j].class_index = j+1;
		csl.costs[j].x = loss(data, label, j+1);
	}
	ec.l.cs = csl;
}

size_t predict_sublearner_noadf(cbify& data, example& ec, uint32_t i)
{
	//For vw's internal reason, we need to first have a cs label before
	//using csoaa to predict
	MULTICLASS::label_t ld = ec.l.multi;
	convert_mc_to_cs(data, ec);
	data.all->cost_sensitive->predict(ec, i);
	ec.l.multi = ld;

	return ec.pred.multiclass;
}


size_t predict_cs(cbify& data, example& ec)
{
	uint32_t argmin = find_min(data.cumulative_costs);
	//cout<<argmin<<endl;
	return predict_sublearner_noadf(data, ec, argmin);
}

void learn_cs(cbify& data, example& ec, size_t ec_type)
{
	float old_weight = ec.weight;
	for (uint32_t i = 0; i < data.choices_lambda; i++)
	{
		float weight_multiplier = compute_weight_multiplier(data, i, ec_type);
		ec.weight = old_weight * weight_multiplier;
		data.all->cost_sensitive->learn(ec, i);
	}
	ec.weight = old_weight;
}

void predict_or_learn_cs(cbify& data, example& ec, size_t ec_type)
{
	MULTICLASS::label_t ld = ec.l.multi;
	//predict
	predict_cs(data, ec);
	data.mc_pred = ec.pred.multiclass;
	//learn
	//first, corrupt fully supervised example ec's label here
	ec.l.multi.label = data.corrupted_label;
	convert_mc_to_cs(data, ec);

	if (ind_update(data, ec_type))
		learn_cs(data, ec, ec_type);

	//set the label of ec back to a multiclass label
	ec.l.multi = ld;
	ec.pred.multiclass = data.mc_pred;
}

void convert_mc_to_cb(cbify& data, example& ec, uint32_t action)
{
	auto& cl = data.cb_label.costs[0];
	cl.action = action;
	cl.probability = ec.pred.a_s[action-1].score;

	if(!cl.action)
		THROW("No action with non-zero probability found!");

	cl.cost = loss(data, data.corrupted_label, action);
	ec.l.cb = data.cb_label;
}


uint32_t predict_bandit(cbify& data, base_learner& base, example& ec)
{
	// we need the cb cost array to be an empty array to make cb prediction
	ec.l.cb.costs = v_init<CB::cb_class>();
	// TODO: not sure why we need the following sentence
	ec.pred.a_s = data.a_s;

	uint32_t argmin = find_min(data.cumulative_costs);
	base.predict(ec, argmin);
	//data.pred = ec.pred;

	uint32_t action = data.mwt_explorer->Choose_Action(*data.generic_explorer, StringUtils::to_string(data.example_counter++), ec);
	ec.l.cb.costs.delete_v();

	return action;

}

void learn_bandit(cbify& data, base_learner& base, example& ec, size_t ec_type)
{
	float old_weight = ec.weight;
	for (uint32_t i = 0; i < data.choices_lambda; i++)
	{
		float weight_multiplier = compute_weight_multiplier(data, i, ec_type);
		ec.weight = old_weight * weight_multiplier;
		base.learn(ec, i);
	}
	ec.weight = old_weight;
}

void predict_or_learn_bandit(cbify& data, base_learner& base, example& ec, size_t ec_type)
{
	MULTICLASS::label_t ld = ec.l.multi;
	uint32_t action = predict_bandit(data, base, ec);
	data.mc_pred = action;

	convert_mc_to_cb(data, ec, action);

	//make sure the prediction here is a cb prediction
	//ec.pred = data.pred;

	if (ind_update(data, ec_type))
		learn_bandit(data, base, ec, ec_type);

	//data.a_s.erase();
	data.a_s = ec.pred.a_s;

	ec.l.multi = ld;
	ec.pred.multiclass = action;
}

void add_to_sup_validation(cbify& data, example& ec)
{
	MULTICLASS::label_t ld = ec.l.multi;
	ec.l.multi.label = data.corrupted_label;
	example* ec_copy = calloc_or_throw<example>(1);
	VW::copy_example_data(false, ec_copy, &ec, 0, MULTICLASS::mc_label.copy_label);
	ec.l.multi = ld;
	// I believe we cannot directly do push_back(ec), as the label won't be deeply copied and that space will be
	// reallocated when the example fall out of the predict_or_learn scope
	data.supervised_validation.push_back(*ec_copy);
	free(ec_copy);
}

void accumulate_costs_ips(cbify& data, example& ec)
{
	CB::cb_class& cl = data.cb_label.costs[0];
	// validation using bandit data
	if (data.validation_method == 1)
	{
		//IPS for approximating the cumulative costs for all lambdas
		for (uint32_t i = 0; i < data.choices_lambda; i++)
		{
			uint32_t action = predict_sublearner_noadf(data, ec, i);

			if (action == cl.action)
				data.cumulative_costs[i] += cl.cost / cl.probability;
			//cout<<data.cumulative_costs[i]<<endl;
		}
		//cout<<endl;
	}
	else //validation using supervised data (their labels are already set to cost-sensitive labels)
	{
		//only update cumulative costs every warm_start_period iterations
		if (abs(log2(data.bandit_iter) - floor(log2(data.bandit_iter))) < 1e-4)
		{
			for (uint32_t i = 0; i < data.choices_lambda; i++)
				data.cumulative_costs[i] = 0;

			//cout<<"updating validation error on supervised data: " << data.bandit_iter / data.warm_start_period << endl;
			for (uint32_t i = 0; i < data.choices_lambda; i++)
			{
				//go over the supervised validation set
				for (uint32_t j = 0; j < data.warm_start_period; j++)
				{
					example& ec_valid = data.supervised_validation[j];
					uint32_t action = predict_sublearner_noadf(data, ec_valid, i);
					data.cumulative_costs[i] += loss(data, ec_valid.l.multi.label, action);
				}
				//cout<<data.cumulative_costs[i]<<endl;
			}
			//cout<<endl;
		}
	}
}

void accumulate_variance(cbify& data, example& ec)
{
	size_t pred_best_approx = predict_cs(data, ec);
	data.cumulative_variance += 1.0 / data.a_s[pred_best_approx-1].score;

	//cout<<"variance at bandit round "<< data.bandit_iter << " = " << 1.0 / data.a_s[pred_best_approx-1].score << endl;
	//cout<<pred_best_approx<<endl;

}

template <bool is_learn>
void predict_or_learn(cbify& data, base_learner& base, example& ec)
{
	//Store the multiclass input label

	//cout<<ld.label<<endl;

	// Initialize the lambda vector
	if (data.warm_start_iter == 0 && data.bandit_iter == 0)
		setup_lambdas(data, ec);

	if (data.warm_start_iter < data.warm_start_period) // Call the cost-sensitive learner directly
	{
		data.corrupted_label = corrupt_action(ec.l.multi.label, data, SUPERVISED);

		if (data.warm_start_type == SUPERVISED_WS)
			predict_or_learn_cs(data, ec, SUPERVISED);
		else
			predict_or_learn_bandit(data, base, ec, SUPERVISED);

		if (data.validation_method == SUPERVISED_VALI)
			add_to_sup_validation(data, ec);

		ec.weight = 0;
		ec.pred.multiclass = data.mc_pred;
		data.warm_start_iter++;
	}
	else if (data.bandit_iter < data.bandit_period) //Call the cb_explore learner. It returns a vector of probabilities for each action
	{
		data.corrupted_label = corrupt_action(ec.l.multi.label, data, BANDIT);
		predict_or_learn_bandit(data, base, ec, BANDIT);
		data.bandit_iter++;
		if (data.bandit_iter == data.bandit_period)
		{
			cout<<"Ideal average variance = "<<data.num_actions / data.epsilon<<endl;
			cout<<"Measured average variance = "<<data.cumulative_variance / data.bandit_period<<endl;
		}

		// accumulate the cumulative costs of lambdas, given data.cb_label has the ips info
		accumulate_costs_ips(data, ec);
		// accumulate the cumulative variances, given we have data.a_s has the score info
		accumulate_variance(data, ec);
		ec.pred.multiclass = data.mc_pred;
	}
	else
	{
		//skipping
		//base.predict(ec, argmin);
		ec.pred.multiclass = 0;
		ec.weight = 0;
	}
}

uint32_t predict_sublearner_adf(cbify& data, base_learner& base, example& ec, uint32_t i)
{
	copy_example_to_adf(data, ec);

	example* ecs = data.adf_data.ecs;
	example* empty = data.adf_data.empty_example;


	for (size_t a = 0; a < data.adf_data.num_actions; ++a)
	{
	  base.predict(ecs[a], i);
		//data.all->cost_sensitive->predict(ecs[a], argmin);
	}
	base.predict(*empty, i);
	//data.all->cost_sensitive->predict(*empty, argmin);

  uint32_t pred_action = ecs[0].pred.a_s[0].action+1;

  //Need to clear the prediction, otherwise there will be a memory leak
  ecs[0].pred.a_s.delete_v();

	return pred_action;
}

size_t predict_cs_adf(cbify& data, base_learner& base, example& ec)
{
	uint32_t argmin = find_min(data.cumulative_costs);
	return predict_sublearner_adf(data, base, ec, argmin);
}


void add_to_sup_validation_adf(cbify& data, example& ec)
{
	//cout<<ec.l.cs.costs.size()<<endl;
	example* ec_copy = calloc_or_throw<example>(1);
	VW::copy_example_data(false, ec_copy, &ec, 0, MULTICLASS::mc_label.copy_label);
	data.supervised_validation.push_back(*ec_copy);
	free(ec_copy);
}


void accumulate_costs_ips_adf(cbify& data, example& ec, CB::cb_class& cl, base_learner& base)
{
	if (data.validation_method == BANDIT_VALI)
	{
		//IPS for approximating the cumulative costs for all lambdas
		for (uint32_t i = 0; i < data.choices_lambda; i++)
		{
			uint32_t action = predict_sublearner_adf(data, base, ec, i);

			if (action == cl.action)
				data.cumulative_costs[i] += cl.cost / cl.probability;
			//cout<<data.cumulative_costs[i]<<endl;
		}
		//cout<<endl;
	}
	else
	{
		//only update cumulative costs every warm_start_period iterations
		if ( abs(log2(data.bandit_iter) - floor(log2(data.bandit_iter))) < 1e-4 )
		{
			for (uint32_t i = 0; i < data.choices_lambda; i++)
				data.cumulative_costs[i] = 0;

			//cout<<"updating validation error on supervised data: " << data.bandit_iter / data.warm_start_period << endl;
			for (uint32_t i = 0; i < data.choices_lambda; i++)
			{
				for (uint32_t j = 0; j < data.warm_start_period; j++)
				{
					example& ec_valid = data.supervised_validation[j];
					uint32_t pred_label = predict_sublearner_adf(data, base, ec_valid, i);
					data.cumulative_costs[i] += loss(data, ec_valid.l.multi.label, pred_label);
					//cout<<ec_valid.l.multi.label<<" "<<pred_label<<endl;
				}
				//cout<<data.cumulative_costs[i]<<endl;
			}
		}
	}
}

void accumulate_variance_adf(cbify& data, base_learner& base, example& ec)
{
	auto& out_ec = data.adf_data.ecs[0];

	data.a_s.erase();
	for (size_t a = 0; a < data.adf_data.num_actions; ++a)
		data.a_s.push_back({out_ec.pred.a_s[a].action, out_ec.pred.a_s[a].score});

	size_t pred_best_approx = predict_cs_adf(data, base, ec);
	float temp_variance;

	for (size_t a = 0; a < data.adf_data.num_actions; ++a)
		if (pred_best_approx == data.a_s[a].action + 1)
			temp_variance = 1.0 / data.a_s[a].score;

	data.cumulative_variance += temp_variance;

	//cout<<"variance at bandit round "<< data.bandit_iter << " = " << temp_variance << endl;
	//cout<<pred_pi<<" "<<pred_best_approx<<" "<<ld.label<<endl;
}

void multiclass_to_cs_adf(cbify& data, COST_SENSITIVE::label* csls, size_t corrupted_label)
{
	for (size_t a = 0; a < data.adf_data.num_actions; ++a)
	{
		csls[a].costs[0].class_index = a+1;
		csls[a].costs[0].x = loss(data, corrupted_label, a+1);
	}

}


void generate_corrupted_cs_adf(cbify& data, MULTICLASS::label_t ld, size_t corrupted_label)
{
	//suppose copy_example_data has already been called
	example* ecs = data.adf_data.ecs;
	example* empty_example = data.adf_data.empty_example;

	//generate cost-sensitive label (only for CSOAA's use - this will be retracted at the end)
	COST_SENSITIVE::label* csls = data.csls;
	COST_SENSITIVE::label* csl_empty = data.csl_empty;

	multiclass_to_cs_adf(data, csls, corrupted_label);

	for (size_t a = 0; a < data.adf_data.num_actions; ++a)
	{
		ecs[a].l.cs = csls[a];
		//cout<<ecs[a].l.cs.costs.size()<<endl;
	}
	empty_example->l.cs = *csl_empty;

}

void learn_cs_adf(cbify& data, size_t ec_type)
{
	example* ecs = data.adf_data.ecs;
	example* empty_example = data.adf_data.empty_example;

	for (size_t a = 0; a < data.adf_data.num_actions; ++a)
		data.old_weights[a] = ecs[a].weight;

	for (uint32_t i = 0; i < data.choices_lambda; i++)
	{
		float weight_multiplier = compute_weight_multiplier(data, i, ec_type);
		for (size_t a = 0; a < data.adf_data.num_actions; ++a)
		{
			ecs[a].weight = data.old_weights[a] * weight_multiplier;
			data.all->cost_sensitive->learn(ecs[a],i);
		}
		data.all->cost_sensitive->learn(*empty_example,i);
	}

	//Seems like we don't need to set the weights back as this example will be
	//discarded anyway
	for (size_t a = 0; a < data.adf_data.num_actions; ++a)
		ecs[a].weight = data.old_weights[a];
}

void predict_or_learn_cs_adf(cbify& data, base_learner& base, example& ec, bool is_update, size_t ec_type)
{
	//Store the multiclass input label
	MULTICLASS::label_t ld = ec.l.multi;

	uint32_t best_action = predict_cs_adf(data, base, ec);

	//data.all->cost_sensitive->predict(ec,argmin);

	//generate cost-sensitive label
	// ecs[a].weight *= 1;
	//				cout << "size cbify = " << ecs[a].l.cs.costs.size() << endl;
	size_t corrupted_label = corrupt_action(ld.label, data, ec_type);
	generate_corrupted_cs_adf(data, ld, corrupted_label);

	if (is_update)
		learn_cs_adf(data, ec_type);

	ec.pred.multiclass = best_action;
	ec.l.multi = ld;

	//a hack here - allocated memories not deleted
	//to be corrected
	if (data.validation_method == SUPERVISED_VALI)
		add_to_sup_validation_adf(data, ec);
}

size_t predict_bandit_adf(cbify& data, base_learner& base)
{
	example* ecs = data.adf_data.ecs;
	example* empty_example = data.adf_data.empty_example;

	uint32_t argmin = find_min(data.cumulative_costs);

	for (size_t a = 0; a < data.adf_data.num_actions; ++a)
	{
		base.predict(ecs[a], argmin);
	}
	base.predict(*empty_example, argmin);

	// get output scores
	auto& out_ec = data.adf_data.ecs[0];
	uint32_t idx = data.mwt_explorer->Choose_Action(
									 *data.generic_explorer,
									 StringUtils::to_string(data.example_counter++), out_ec) - 1;

	return idx;

}

void generate_corrupted_cb_adf(cbify& data, CB::cb_class& cl, MULTICLASS::label_t& ld, size_t idx, size_t corrupted_label)
{
	auto& out_ec = data.adf_data.ecs[0];
	cl.action = out_ec.pred.a_s[idx].action + 1;
	cl.probability = out_ec.pred.a_s[idx].score;

	if(!cl.action)
		THROW("No action with non-zero probability found!");

	cl.cost = loss(data, corrupted_label, cl.action);

}

void learn_bandit_adf(cbify& data, base_learner& base, size_t ec_type)
{
	example* ecs = data.adf_data.ecs;
	example* empty_example = data.adf_data.empty_example;

	for (size_t a = 0; a < data.adf_data.num_actions; ++a)
		data.old_weights[a] = ecs[a].weight;

	for (uint32_t i = 0; i < data.choices_lambda; i++)
	{
		float weight_multiplier = compute_weight_multiplier(data, i, ec_type);
		for (size_t a = 0; a < data.adf_data.num_actions; ++a)
		{
			ecs[a].weight = data.old_weights[a] * weight_multiplier;
			base.learn(ecs[a], i);
		}
		base.learn(*empty_example, i);
	}

	for (size_t a = 0; a < data.adf_data.num_actions; ++a)
		ecs[a].weight = data.old_weights[a];
}

void predict_or_learn_bandit_adf(cbify& data, base_learner& base, example& ec, bool is_update, size_t ec_type)
{
	//Store the multiclass input label
	MULTICLASS::label_t ld = ec.l.multi;

	//copy_example_to_adf(data, ec);

	//for (size_t a = 0; a < data.adf_data.num_actions; ++a)
	//	data.cbls[a].costs = data.adf_data.ecs[a].l.cb.costs;
	//data.cbl_empty->costs = data.adf_data.empty_example->l.cb.costs;

	//size_t pred_pi = predict_cs_adf(data, base, ec);
	uint32_t idx = predict_bandit_adf(data, base);

	CB::cb_class cl;

	size_t corrupted_label = corrupt_action(ld.label, data, ec_type);
	generate_corrupted_cb_adf(data, cl, ld, idx, corrupted_label);

	// accumulate the cumulative costs of lambdas
	accumulate_costs_ips_adf(data, ec, cl, base);

	// add cb label to chosen action
	auto& lab = data.adf_data.ecs[cl.action - 1].l.cb;
	lab.costs.push_back(cl);

	if (is_update)
		learn_bandit_adf(data, base, ec_type);

	accumulate_variance_adf(data, base, ec);

  lab.costs.delete_v();
	ec.pred.multiclass = cl.action;
}


template <bool is_learn>
void predict_or_learn_adf(cbify& data, base_learner& base, example& ec)
{
	if (data.warm_start_iter == 0 && data.bandit_iter == 0)
		setup_lambdas(data, ec);

  copy_example_to_adf(data, ec);

  // As we will be processing the examples with cs or cb labels,
  // we need to store the default cb label so that the next time we call copy_example_to_adf
  // we can free it successfully (that is the whole purpose of data.cbls)
  for (size_t a = 0; a < data.adf_data.num_actions; ++a)
    data.cbls[a].costs = data.adf_data.ecs[a].l.cb.costs;
  data.cbl_empty->costs = data.adf_data.empty_example->l.cb.costs;

	if (data.warm_start_iter < data.warm_start_period) // Call the cost-sensitive learner directly
	{
		if (data.warm_start_type == SUPERVISED_WS)
			predict_or_learn_cs_adf(data, base, ec, data.ind_supervised, SUPERVISED);
		else
			predict_or_learn_bandit_adf(data, base, ec, data.ind_supervised, SUPERVISED);
		ec.weight = 0;
		data.warm_start_iter++;
	}
	else if (data.bandit_iter < data.bandit_period) // call the bandit learner
	{
		predict_or_learn_bandit_adf(data, base, ec, data.ind_bandit, BANDIT);
		data.bandit_iter++;
		if (data.bandit_iter == data.bandit_period)
		{
			cout<<"Ideal average variance = "<<data.num_actions / data.epsilon<<endl;
			cout<<"Measured average variance = "<<data.cumulative_variance / data.bandit_period<<endl;
		}
	}
	else
	{
		ec.pred.multiclass = 0;
		ec.weight = 0;
	}

	for (size_t a = 0; a < data.adf_data.num_actions; ++a)
		data.adf_data.ecs[a].l.cb.costs = data.cbls[a].costs;
	data.adf_data.empty_example->l.cb.costs = data.cbl_empty->costs;
}

void init_adf_data(cbify& data, const size_t num_actions)
{
  auto& adf_data = data.adf_data;
  adf_data.num_actions = num_actions;

  adf_data.ecs = VW::alloc_examples(CB::cb_label.label_size, num_actions);
  adf_data.empty_example = VW::alloc_examples(CB::cb_label.label_size, 1);
  for (size_t a=0; a < num_actions; ++a)
  {
    auto& lab = adf_data.ecs[a].l.cb;
    CB::cb_label.default_label(&lab);
  }
  CB::cb_label.default_label(&adf_data.empty_example->l.cb);
  adf_data.empty_example->in_use = true;
	adf_data.empty_example->pred.a_s = v_init<action_score>();

	data.csls = calloc_or_throw<COST_SENSITIVE::label>(num_actions);
	data.csl_empty = calloc_or_throw<COST_SENSITIVE::label>(1);

	data.cbls = calloc_or_throw<CB::label>(num_actions);
	data.cbl_empty = calloc_or_throw<CB::label>(1);

	data.old_weights = calloc_or_throw<float>(num_actions);

	data.csl_empty->costs = v_init<COST_SENSITIVE::wclass>();
	data.csl_empty->costs.push_back({0, 0, 0, 0});
	data.csl_empty->costs[0].class_index = 0;
	data.csl_empty->costs[0].x = FLT_MAX;

	for (size_t a = 0; a < num_actions; ++a)
	{
		data.csls[a].costs = v_init<COST_SENSITIVE::wclass>();
		data.csls[a].costs.push_back({0, a+1, 0, 0});
		//cout<<data.csls[a].costs.size()<<endl;
	}

}


base_learner* cbify_setup(vw& all)
{
  //parse and set arguments
  if (missing_option<size_t, true>(all, "cbify", "Convert multiclass on <k> classes into a contextual bandit problem"))
    return nullptr;
  new_options(all, "CBIFY options")
  ("loss0", po::value<float>(), "loss for correct label")
  ("loss1", po::value<float>(), "loss for incorrect label")
	("warm_start", po::value<size_t>(), "number of training examples for warm start")
	("bandit", po::value<size_t>(), "number of training examples for bandit processing")
  ("choices_lambda", po::value<size_t>(), "numbers of lambdas importance weights to aggregate")
	("no_supervised", "indicator of using supervised only")
	("no_bandit", "indicator of using bandit only")
	("corrupt_prob_supervised", po::value<float>(), "probability of label corruption in the supervised part")
	("corrupt_prob_bandit", po::value<float>(), "probability of label corruption in the bandit part")
	("corrupt_type_supervised", po::value<size_t>(), "type of label corruption in the supervised part (1 is uar, 2 is circular)")
	("corrupt_type_bandit", po::value<size_t>(), "probability of label corruption in the bandit part (1 is uar, 2 is circular)")
	("validation_method", po::value<size_t>(), "lambda selection criterion (1 is using bandit with progressive validation, 2 is using supervised)")
	("weighting_scheme", po::value<size_t>(), "weighting scheme (1 is per instance weighting, 2 is per dataset weighting (where we use a diminishing weighting scheme) )")
	("lambda_scheme", po::value<size_t>(), "Lambda set scheme (1 is expanding based on center 0.5, 2 is expanding based on center=minimax lambda, 3 is expanding based on center=minimax lambda along with forcing 0,1 in Lambda )")
	("overwrite_label", po::value<size_t>(), "the label type 3 corruptions (overwriting) turn to")
	("warm_start_type", po::value<size_t>(), "the type of warm start approach (1 is supervised warm start, 2 is contextual bandit warm start)");
  add_options(all);

  po::variables_map& vm = all.vm;
  uint32_t num_actions = (uint32_t)vm["cbify"].as<size_t>();

  cbify& data = calloc_or_throw<cbify>();
  data.use_adf = count(all.args.begin(), all.args.end(),"--cb_explore_adf") > 0;
  data.loss0 = vm.count("loss0") ? vm["loss0"].as<float>() : 0.f;
  data.loss1 = vm.count("loss1") ? vm["loss1"].as<float>() : 1.f;
	data.ind_supervised = vm.count("no_supervised") ? false : true;
	data.ind_bandit = vm.count("no_bandit") ? false : true;

  data.recorder = new vw_recorder();
  data.mwt_explorer = new MwtExplorer<example>("vw",*data.recorder);
  data.scorer = new vw_scorer();
  data.a_s = v_init<action_score>();
  //data.probs = v_init<float>();
  data.generic_explorer = new GenericExplorer<example>(*data.scorer, (u32)num_actions);
  data.all = &all;


	//cout<<data.warm_start_period<<endl;
	data.warm_start_period = vm.count("warm_start") ? vm["warm_start"].as<size_t>() : 0;
	data.bandit_period = vm.count("bandit") ?  vm["bandit"].as<size_t>() : UINT32_MAX; //ideally should be the size of the dataset

	//cout<<data.warm_start_period<<endl;
	data.choices_lambda = vm.count("choices_lambda") ? vm["choices_lambda"].as<size_t>() : 1;

	data.corrupt_prob_supervised = vm.count("corrupt_prob_supervised") ? vm["corrupt_prob_supervised"].as<float>() : 0.0;
	data.corrupt_prob_bandit = vm.count("corrupt_prob_bandit") ? vm["corrupt_prob_bandit"].as<float>() : 0.0;
	data.corrupt_type_supervised = vm.count("corrupt_type_supervised") ? vm["corrupt_type_supervised"].as<size_t>() : UAR; // 1 is the default value
	data.corrupt_type_bandit = vm.count("corrupt_type_bandit") ? vm["corrupt_type_bandit"].as<size_t>() : UAR; // 1 is the default value
	data.validation_method = vm.count("validation_method") ? vm["validation_method"].as<size_t>() : BANDIT_VALI; // 1 is the default value
	data.weighting_scheme = vm.count("weighting_scheme") ? vm["weighting_scheme"].as<size_t>() : INSTANCE_WT; // 1 is the default value
	data.lambda_scheme = vm.count("lambda_scheme") ? vm["lambda_scheme"].as<size_t>() : ABS_CENTRAL;
	data.epsilon = vm.count("epsilon") ? vm["epsilon"].as<float>() : 0.05;
	data.overwrite_label = vm.count("overwrite_label") ? vm["overwrite_label"].as<size_t>() : 1;
	data.warm_start_type = vm.count("warm_start_type") ? vm["warm_start_type"].as<size_t>() : SUPERVISED_WS;
	//cout<<"does epsilon exist?"<<vm.count("epsilon")<<endl;
	//cout<<"epsilon = "<<data.epsilon<<endl;

	if (data.validation_method == SUPERVISED_VALI)
	{
		data.supervised_validation = v_init<example>();
		//calloc_or_throw<example>(data.warm_start_period);
	}


	data.bandit_iter = 0;
	data.warm_start_iter = 0;


	//generate_lambdas(data.lambdas, data.choices_lambda);

	data.lambdas = v_init<float>();
	for (uint32_t i = 0; i < data.choices_lambda; i++)
		data.lambdas.push_back(0.);

	for (size_t i = 0; i < data.choices_lambda; i++)
		data.cumulative_costs.push_back(0.);

	data.cumulative_variance = 0;

	data.num_actions = num_actions;


  if (data.use_adf)
  {
    init_adf_data(data, num_actions);
  }
	else
	{
		//data.csls = calloc_or_throw<COST_SENSITIVE::label>(1);
		//auto& csl = data.csls[0];

		data.cs_label.costs = v_init<COST_SENSITIVE::wclass>();
		//Note: these two lines are important, otherwise the cost sensitive vector seems to be unbounded.

		for (size_t a = 0; a < num_actions; ++a)
			data.cs_label.costs.push_back({0, a+1, 0, 0});

		data.cb_label.costs.push_back({0, 1, 0, 0});
	}


  if (count(all.args.begin(), all.args.end(),"--cb_explore") == 0 && !data.use_adf)
  {
    all.args.push_back("--cb_explore");
    stringstream ss;
    ss << num_actions;
    all.args.push_back(ss.str());
  }
  if (count(all.args.begin(), all.args.end(), "--baseline"))
  {
    all.args.push_back("--lr_multiplier");
    stringstream ss;
    ss << max<float>(abs(data.loss0), abs(data.loss1)) / (data.loss1 - data.loss0);
    all.args.push_back(ss.str());
  }
  base_learner* base = setup_base(all);

  all.delete_prediction = nullptr;
  learner<cbify>* l;
  if (data.use_adf)
  {
    l = &init_multiclass_learner(&data, base, predict_or_learn_adf<true>, predict_or_learn_adf<false>, all.p, data.choices_lambda);
  }
  else
  {
    l = &init_multiclass_learner(&data, base, predict_or_learn<true>, predict_or_learn<false>, all.p, data.choices_lambda);
  }
  l->set_finish(finish);

  return make_base(*l);
}

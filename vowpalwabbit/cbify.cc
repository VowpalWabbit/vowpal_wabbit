#include <float.h>
#include "reductions.h"
#include "cb_algs.h"
#include "rand48.h"
#include "bs.h"
#include "vw.h"
#include "../explore/hash.h"
#include "explore.h"

#include <vector>

using namespace LEARNER;
using namespace exploration;
using namespace ACTION_SCORE;
using namespace std;

#define WARM_START 1
#define INTERACTION 2

#define SUPERVISED_WS 1
#define BANDIT_WS 2

#define UAR 1
#define CIRCULAR 2
#define OVERWRITE 3

#define INTER_VALI 1
#define WS_VALI_SPLIT 2
#define WS_VALI_NOSPLIT 3

#define INSTANCE_WT 1
#define DATASET_WT 2

#define ABS_CENTRAL 1
#define ABS_CENTRAL_ZEROONE 2
#define MINIMAX_CENTRAL 3
#define MINIMAX_CENTRAL_ZEROONE 4


struct cbify;

struct cbify_adf_data
{
  example* ecs;
  example* empty_example;
  size_t num_actions;
};

struct cbify
{
  CB::label cb_label;
  uint64_t app_seed;
  action_scores a_s;
  // used as the seed
  size_t example_counter;
  vw* all;
  bool use_adf; // if true, reduce to cb_explore_adf instead of cb_explore
  cbify_adf_data adf_data;
  float loss0;
  float loss1;

	//warm start parameters
	uint32_t ws_period;
	uint32_t inter_period;
	uint32_t choices_lambda;
	bool upd_ws;
	bool upd_inter;
	int cor_type_ws;
	float cor_prob_ws;
	int cor_type_inter;
	float cor_prob_inter;
	int vali_method;
	int wt_scheme;
	int lambda_scheme;
	uint32_t overwrite_label;
	int ws_type;

	//auxiliary variables
	uint32_t num_actions;
	float epsilon;
	vector<float> lambdas;
	action_scores a_s_adf;
	vector<float> cumulative_costs;
	CB::cb_class cl_adf;
	uint32_t ws_train_size;
	uint32_t ws_vali_size;
	vector<example> ws_vali;
	float cumu_var;
	uint32_t ws_iter;
	uint32_t inter_iter;
	MULTICLASS::label_t mc_label;
	COST_SENSITIVE::label* csls;
	COST_SENSITIVE::label* csl_empty;

};

float loss(cbify& data, uint32_t label, uint32_t final_prediction)
{
  if (label != final_prediction)
    return data.loss1;
  else
    return data.loss0;
}

template<class T> inline void delete_it(T* p) { if (p != nullptr) delete p; }

template <class T>
uint32_t find_min(vector<T> arr)
{
	T min_val = FLT_MAX;
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
  CB::cb_label.delete_label(&data.cb_label);
  data.a_s.delete_v();
  if (data.use_adf)
  {
		cout<<"The average variance estimate is:"<<data.cumu_var / data.inter_period<<endl;
		cout<<"The theoretical average variance is:"<<data.num_actions / data.epsilon<<endl;
		uint32_t argmin = find_min(data.cumulative_costs);
		cout<<"The last value of lambda chosen is:"<<data.lambdas[argmin]<<endl;

    for (size_t a = 0; a < data.adf_data.num_actions; ++a)
    {
      VW::dealloc_example(CB::cb_label.delete_label, data.adf_data.ecs[a]);
    }
    VW::dealloc_example(CB::cb_label.delete_label, *data.adf_data.empty_example);
    free(data.adf_data.ecs);
    free(data.adf_data.empty_example);
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
    if (CB_ALGS::example_is_newline_not_header(eca) && CB::cb_label.test_label(&eca.l))
    {
      eca.tag.push_back('n');
    }
  }
}

float minimax_lambda(float epsilon, size_t num_actions, size_t warm_start_period, size_t interaction_period)
{
	return epsilon / (num_actions + epsilon);
}

void setup_lambdas(cbify& data)
{
	// The lambdas are in fact arranged in ascending order (the 'middle' lambda is 0.5)
	vector<float>& lambdas = data.lambdas;

	//interaction only
	if (!data.upd_ws && data.upd_inter)
	{
		for (uint32_t i = 0; i<data.choices_lambda; i++)
			lambdas[i] = 1.0;
		return;
	}

	//warm start only
	if (!data.upd_inter && data.upd_ws)
	{
		for (uint32_t i = 0; i<data.choices_lambda; i++)
			lambdas[i] = 0.0;
		return;
	}

	//if no warm start and no interaction, then as there are no updates anyway,
	//we are still fine

	uint32_t mid = data.choices_lambda / 2;

	if (data.lambda_scheme == ABS_CENTRAL || data.lambda_scheme == ABS_CENTRAL_ZEROONE)
		lambdas[mid] = 0.5;
	else
		lambdas[mid] = minimax_lambda(data.epsilon, data.num_actions, data.ws_period, data.inter_period);

	for (uint32_t i = mid; i > 0; i--)
		lambdas[i-1] = lambdas[i] / 2;

	for (uint32_t i = mid+1; i < data.choices_lambda; i++)
		lambdas[i] = 1 - (1-lambdas[i-1]) / 2;

	if (data.lambda_scheme == MINIMAX_CENTRAL_ZEROONE || data.lambda_scheme == ABS_CENTRAL_ZEROONE)
	{
		lambdas[0] = 0.0;
		lambdas[data.choices_lambda-1] = 1.0;
	}
}

uint32_t generate_uar_action(cbify& data)
{
	float randf = merand48(data.all->random_state);

	for (uint32_t i = 1; i <= data.num_actions; i++)
	{
		if (randf <= float(i) / data.num_actions)
			return i;
	}
	return data.num_actions;
}

uint32_t corrupt_action(cbify& data, uint32_t action, int ec_type)
{
	float cor_prob;
	uint32_t cor_type;
	uint32_t cor_action;

	if (ec_type == WARM_START)
	{
		cor_prob = data.cor_prob_ws;
		cor_type = data.cor_type_ws;
	}
	else
	{
		cor_prob = data.cor_prob_inter;
		cor_type = data.cor_type_inter;
	}

	float randf = merand48(data.all->random_state);
	if (randf < cor_prob)
	{
		if (cor_type == UAR)
			cor_action = generate_uar_action(data);
		else if (cor_type == OVERWRITE)
			cor_action = data.overwrite_label;
		else
			cor_action = (action % data.num_actions) + 1;
	}
	else
		cor_action = action;
	return cor_action;
}

bool ind_update(cbify& data, int ec_type)
{
	if (ec_type == WARM_START)
		return data.upd_ws;
	else
		return data.upd_inter;
}

float compute_weight_multiplier(cbify& data, size_t i, int ec_type)
{
	float weight_multiplier;
	float ws_train_size = data.ws_train_size;
	float inter_train_size = data.inter_period;

	if (data.vali_method != INTER_VALI)
	{
		if (ec_type == WARM_START && data.ws_iter >= ws_train_size)
			return 0.0;
	}

	float total_train_size = ws_train_size + inter_train_size;
	if (data.wt_scheme == INSTANCE_WT)
	{
		if (ec_type == WARM_START)
			weight_multiplier = (1-data.lambdas[i]) * total_train_size / ws_train_size;
		else
			weight_multiplier = data.lambdas[i] * total_train_size / inter_train_size;
	}
	else
	{
		float total_weight = (1-data.lambdas[i]) * ws_train_size + data.lambdas[i] * inter_train_size;

		if (ec_type == WARM_START)
			weight_multiplier = (1-data.lambdas[i]) * total_train_size / total_weight;
		else
			weight_multiplier = data.lambdas[i] * total_train_size / total_weight;
	}
	return weight_multiplier;
}


template <bool is_learn>
void predict_or_learn(cbify& data, single_learner& base, example& ec)
{
  //Store the multiclass input label
  MULTICLASS::label_t ld = ec.l.multi;
  data.cb_label.costs.clear();
  ec.l.cb = data.cb_label;
  ec.pred.a_s = data.a_s;

  //Call the cb_explore algorithm. It returns a vector of probabilities for each action
  base.predict(ec);
  //data.probs = ec.pred.scalars;

  uint32_t chosen_action;
  if (sample_after_normalizing(data.app_seed + data.example_counter++, begin_scores(ec.pred.a_s), end_scores(ec.pred.a_s), chosen_action))
    THROW("Failed to sample from pdf");

  CB::cb_class cl;
  cl.action = chosen_action + 1;
  cl.probability = ec.pred.a_s[chosen_action].score;

  if(!cl.action)
    THROW("No action with non-zero probability found!");
  cl.cost = loss(data, ld.label, cl.action);

  //Create a new cb label
  data.cb_label.costs.push_back(cl);
  ec.l.cb = data.cb_label;
  base.learn(ec);
  data.a_s.clear();
  data.a_s = ec.pred.a_s;
  ec.l.multi = ld;
  ec.pred.multiclass = chosen_action + 1;
}

uint32_t predict_sublearner_adf(cbify& data, single_learner& base, example& ec, uint32_t i)
{
	copy_example_to_adf(data, ec);

	example* ecs = data.adf_data.ecs;
	example* empty = data.adf_data.empty_example;

	for (size_t a = 0; a < data.adf_data.num_actions; ++a)
	  base.predict(ecs[a], i);
	base.predict(*empty, i);

	return ecs[0].pred.a_s[0].action+1;
}

void accumu_costs_iv_adf(cbify& data, single_learner& base, example& ec)
{
	CB::cb_class& cl = data.cl_adf;
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

void accumu_costs_wsv_adf(cbify& data, single_learner& base)
{
	uint32_t ws_vali_size = data.ws_vali_size;
	//only update cumulative costs every warm_start_period iterations
	if ( data.inter_iter >= 1 && abs( log2(data.inter_iter+1) - floor(log2(data.inter_iter+1)) ) < 1e-4 )
	{
		for (uint32_t i = 0; i < data.choices_lambda; i++)
			data.cumulative_costs[i] = 0;

		uint32_t num_epochs = ceil(log2(data.inter_period));
		uint32_t epoch = log2(data.inter_iter+1) - 1;
		float batch_vali_size = ((float) ws_vali_size) / num_epochs;
		uint32_t lb, ub;

		if (data.vali_method == WS_VALI_SPLIT)
		{
			lb = ceil(batch_vali_size * epoch);
			ub = ceil(batch_vali_size * (epoch + 1));
		}
		else
		{
			lb = 0;
			ub = ws_vali_size;
		}
		//cout<<"updating validation error on supervised data: " << data.bandit_iter / data.warm_start_period << endl;
		for (uint32_t i = 0; i < data.choices_lambda; i++)
		{
			for (uint32_t j = lb; j < ub; j++)
			{
				example& ec_vali = data.ws_vali[j];
				uint32_t pred_label = predict_sublearner_adf(data, base, ec_vali, i);
				data.cumulative_costs[i] += loss(data, ec_vali.l.multi.label, pred_label);
				//cout<<ec_vali.l.multi.label<<" "<<pred_label<<endl;
			}
			//cout<<data.cumulative_costs[i]<<endl;
		}
	}
}

void add_to_vali(cbify& data, example& ec)
{
	//if this does not work, we can try declare ws_vali as an array
	example ec_copy;
	VW::copy_example_data(false, &ec_copy, &ec, 0, MULTICLASS::mc_label.copy_label);
	data.ws_vali.push_back(ec_copy);
}

uint32_t predict_cs_adf(cbify& data, single_learner& base, example& ec)
{
	uint32_t argmin = find_min(data.cumulative_costs);
	return predict_sublearner_adf(data, base, ec, argmin);
}

void learn_cs_adf(cbify& data, single_learner& base, example& ec, int ec_type)
{
	//generate cost-sensitive label (only for CSOAA's use - this will be retracted at the end)
	auto& csls = data.csls;
	auto& csl_empty = data.csl_empty;

	for (size_t a = 0; a < data.adf_data.num_actions; ++a)
	{
		csls[a].costs[0].class_index = a+1;
		csls[a].costs[0].x = loss(data, ec.l.multi.label, a+1);
	}

	copy_example_to_adf(data, ec);
	example* ecs = data.adf_data.ecs;
	example* empty_example = data.adf_data.empty_example;

	for (size_t a = 0; a < data.adf_data.num_actions; ++a)
	{
		ecs[a].l.cs = csls[a];
		//cout<<ecs[a].l.cs.costs.size()<<endl;
	}
	empty_example->l.cs = *csl_empty;

	vector<float> old_weights;
	for (size_t a = 0; a < data.adf_data.num_actions; ++a)
		old_weights[a] = ecs[a].weight;
	for (uint32_t i = 0; i < data.choices_lambda; i++)
	{
		float weight_multiplier = compute_weight_multiplier(data, i, ec_type);
		for (size_t a = 0; a < data.adf_data.num_actions; ++a)
		{
			ecs[a].weight = old_weights[a] * weight_multiplier;
			data.all->cost_sensitive->learn(ecs[a],i);
		}
		data.all->cost_sensitive->learn(*empty_example,i);
	}
	//Seems like we don't need to set the weights back as this example will be
	//discarded anyway
	for (size_t a = 0; a < data.adf_data.num_actions; ++a)
		ecs[a].weight = old_weights[a];
}

void predict_or_learn_cs_adf(cbify& data, single_learner& base, example& ec, int ec_type)
{
	uint32_t action = predict_cs_adf(data, base, ec);

	if (ind_update(data, ec_type))
		learn_cs_adf(data, base, ec, ec_type);

	ec.pred.multiclass = action;
}


uint32_t predict_bandit_adf(cbify& data, single_learner& base, example& ec)
{
	uint32_t argmin = find_min(data.cumulative_costs);

  copy_example_to_adf(data, ec);
  for (size_t a = 0; a < data.adf_data.num_actions; ++a)
  {
    base.predict(data.adf_data.ecs[a], argmin);
  }
  base.predict(*data.adf_data.empty_example, argmin);

  auto& out_ec = data.adf_data.ecs[0];

  uint32_t chosen_action;
  if (sample_after_normalizing(data.app_seed + data.example_counter++, begin_scores(out_ec.pred.a_s), end_scores(out_ec.pred.a_s), chosen_action))
    THROW("Failed to sample from pdf");

	auto& a_s = data.a_s_adf;
	copy_array<action_score>(a_s, out_ec.pred.a_s);

	auto& cl = data.cl_adf;
  cl.action = a_s[chosen_action].action + 1;
  cl.probability = a_s[chosen_action].score;

  if(!cl.action)
    THROW("No action with non-zero probability found!");
  cl.cost = loss(data, ec.l.multi.label, cl.action);

	ec.pred.multiclass = cl.action;

	return chosen_action;
}

void learn_bandit_adf(cbify& data, single_learner& base, example& ec, int ec_type, uint32_t chosen_action, action_scores& a_s)
{
	//Store the multiclass input label
  MULTICLASS::label_t ld = ec.l.multi;

	copy_example_to_adf(data, ec);
	example* ecs = data.adf_data.ecs;
	example* empty_example = data.adf_data.empty_example;

  // add cb label to chosen action
	auto& cl = data.cl_adf;
  auto& lab = data.adf_data.ecs[cl.action - 1].l.cb;
  lab.costs.push_back(cl);

	vector<float> old_weights;
	for (size_t a = 0; a < data.adf_data.num_actions; ++a)
		old_weights[a] = data.adf_data.ecs[a].weight;

	for (uint32_t i = 0; i < data.choices_lambda; i++)
	{
		float weight_multiplier = compute_weight_multiplier(data, i, ec_type);
	  for (size_t a = 0; a < data.adf_data.num_actions; ++a)
	  {
			ecs[a].weight = old_weights[a] * weight_multiplier;
	    base.learn(ecs[a]);
	  }
	  base.learn(*empty_example);
	}

	for (size_t a = 0; a < data.adf_data.num_actions; ++a)
		data.adf_data.ecs[a].weight = old_weights[a];

  //ec.pred.multiclass = cl.action;
}

void predict_or_learn_bandit_adf(cbify& data, single_learner& base, example& ec, int ec_type)
{
	uint32_t action = predict_bandit_adf(data, base, ec);

	if (ec_type == INTERACTION && data.vali_method == INTER_VALI)
		accumu_costs_iv_adf(data, base, ec);

	if (ind_update(data, ec_type))
		learn_bandit_adf(data, base, ec, ec_type, action, data.a_s_adf);

	if (ec_type == INTERACTION && (data.vali_method == WS_VALI_SPLIT || data.vali_method == WS_VALI_NOSPLIT))
		accumu_costs_wsv_adf(data, base);

	ec.pred.multiclass = action;
}

void accumu_var_adf(cbify& data, single_learner& base, example& ec)
{
	size_t pred_best_approx = predict_cs_adf(data, base, ec);
	float temp_var;

	for (size_t a = 0; a < data.adf_data.num_actions; ++a)
		if (pred_best_approx == data.a_s_adf[a].action + 1)
			temp_var = 1.0 / data.a_s_adf[a].score;

	data.cumu_var += temp_var;

	//cout<<"variance at bandit round "<< data.bandit_iter << " = " << temp_variance << endl;
	//cout<<pred_pi<<" "<<pred_best_approx<<" "<<ld.label<<endl;
}

template <bool is_learn>
void predict_or_learn_adf(cbify& data, single_learner& base, example& ec)
{
	if (data.ws_iter < data.ws_period)
	{
		data.mc_label = ec.l.multi;
		ec.l.multi.label = corrupt_action(data, data.mc_label.label, WARM_START);
		if (data.ws_iter < data.ws_train_size)
		{
			if (data.ws_type == SUPERVISED_WS)
				predict_or_learn_cs_adf(data, base, ec, WARM_START);
			else if (data.ws_type == BANDIT_WS)
				predict_or_learn_bandit_adf(data, base, ec, WARM_START);
		}
		else
			add_to_vali(data, ec);

		ec.l.multi = data.mc_label;
		ec.weight = 0;
		data.ws_iter++;
	}
	else if (data.inter_iter < data.inter_period)
	{
		data.mc_label = ec.l.multi;
		ec.l.multi.label = corrupt_action(data, data.mc_label.label, INTERACTION);
		predict_or_learn_bandit_adf(data, base, ec, INTERACTION);
		accumu_var_adf(data, base, ec);
		ec.l.multi = data.mc_label;
		data.inter_iter++;
	}
	else
	{
		ec.weight = 0;
	}
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

	data.csls = calloc_or_throw<COST_SENSITIVE::label>(num_actions);
	data.csl_empty = calloc_or_throw<COST_SENSITIVE::label>(1);
	for (uint32_t a=0; a < num_actions; ++a)
	{
		COST_SENSITIVE::cs_label.default_label(&data.csls[a]);
		data.csls[a].costs.push_back({0, a+1, 0, 0});
	}
	COST_SENSITIVE::cs_label.default_label(data.csl_empty);
}

base_learner* cbify_setup(arguments& arg)
{
  uint32_t num_actions=0;
  auto data = scoped_calloc_or_throw<cbify>();

  if (arg.new_options("Make Multiclass into Contextual Bandit")
      .critical("cbify", num_actions, "Convert multiclass on <k> classes into a contextual bandit problem")
      ("loss0", data->loss0, 0.f, "loss for correct label")
      ("loss1", data->loss1, 1.f, "loss for incorrect label")
			("epsilon", data->epsilon, 0.05f, "greedy probability")
			("warm_start", data->ws_period, 0U, "number of training examples for warm start")
			("interaction", data->inter_period, 0U, "number of training examples for bandit processing")
		  ("choices_lambda", data->choices_lambda, 1U, "numbers of lambdas importance weights to aggregate")
			("warm_start_update", data->upd_ws, true, "indicator of warm start updates")
			("interaction_update", data->upd_inter, true, "indicator of interaction updates")
			("corrupt_type_warm_start", data->cor_type_ws, UAR, "type of label corruption in the warm start phase (1: uniformly at random, 2: circular, 3: replacing with overwriting label)")
			("corrupt_prob_warm_start", data->cor_prob_ws, 0.f, "probability of label corruption in the warm start phase")
			("corrupt_type_bandit", data->cor_type_inter, UAR, "type of label corruption in the interaction phase (1: uniformly at random, 2: circular, 3: replacing with overwriting label)")
			("corrupt_prob_bandit", data->cor_prob_inter, 0.f, "probability of label corruption in the interaction phase")
			("validation_method", data->vali_method, INTER_VALI, "lambda selection criterion (1 is using bandit with progressive validation, 2 is using supervised)")
			("weighting_scheme", data->wt_scheme, INSTANCE_WT, "weighting scheme (1 is per instance weighting, 2 is per dataset weighting (where we use a diminishing weighting scheme) )")
			("lambda_scheme", data->lambda_scheme, ABS_CENTRAL, "Lambda set scheme (1 is expanding based on center=0.5, 2 is expanding based on center=0.5 and enforcing 0,1 in Lambda, 3 is expanding based on center=minimax lambda, 4 is expanding based on center=minimax lambda and enforcing 0,1 in Lambda )")
			("overwrite_label", data->overwrite_label, 1U, "the label type 3 corruptions (overwriting) turn to")
			("warm_start_type", data->ws_type, SUPERVISED_WS, "the way of utilizing warm start data (1 is using supervised updates, 2 is using contextual bandit updates)").missing())
    return nullptr;

  data->use_adf = count(arg.args.begin(), arg.args.end(),"--cb_explore_adf") > 0;
  data->app_seed = uniform_hash("vw", 2, 0);
  data->a_s = v_init<action_score>();
  data->all = arg.all;

	data->num_actions = num_actions;

  if (data->use_adf)
    init_adf_data(*data.get(), num_actions);

	if (data->vali_method == WS_VALI_SPLIT || data->vali_method == WS_VALI_NOSPLIT)
	{
		data->ws_train_size = ceil(data->ws_period / 2.0);
		data->ws_vali_size = data->ws_period - data->ws_train_size;
	}
	else
	{
		data->ws_train_size = data->ws_period;
		data->ws_vali_size = 0;
	}

  if (count(arg.args.begin(), arg.args.end(),"--cb_explore") == 0 && !data->use_adf)
  {
    arg.args.push_back("--cb_explore");
    stringstream ss;
    ss << num_actions;
    arg.args.push_back(ss.str());
  }
  if (count(arg.args.begin(), arg.args.end(), "--baseline"))
  {
    arg.args.push_back("--lr_multiplier");
    stringstream ss;
    ss << max<float>(abs(data->loss0), abs(data->loss1)) / (data->loss1 - data->loss0);
    arg.args.push_back(ss.str());
  }
  auto base = as_singleline(setup_base(arg));

  arg.all->delete_prediction = nullptr;
  learner<cbify,example>* l;
  if (data->use_adf)
    l = &init_multiclass_learner(data, base, predict_or_learn_adf<true>, predict_or_learn_adf<false>, arg.all->p, data->choices_lambda);
  else
    l = &init_multiclass_learner(data, base, predict_or_learn<true>, predict_or_learn<false>, arg.all->p, 1);
  l->set_finish(finish);

  return make_base(*l);
}

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
#define SKIP 3

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
  multi_ex ecs;
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
	vector<example*> ws_vali;
	float cumu_var;
	uint32_t ws_iter;
	uint32_t inter_iter;
	MULTICLASS::label_t mc_label;
	COST_SENSITIVE::label cs_label;
	COST_SENSITIVE::label* csls;
	CB::label* cbls;
	bool use_cs;

};

float loss(cbify& data, uint32_t label, uint32_t final_prediction)
{
  if (label != final_prediction)
    return data.loss1;
  else
    return data.loss0;
}

float loss_cs(cbify& data, v_array<COST_SENSITIVE::wclass>& costs, uint32_t final_prediction)
{
  float cost = 0.;
  for (auto wc : costs)
  { if (wc.class_index == final_prediction)
    { cost = wc.x;
      break;
    }
  }
  return data.loss0 + (data.loss1 - data.loss0) * cost;
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
		cout<<"average variance estimate = "<<data.cumu_var / data.inter_iter<<endl;
		cout<<"theoretical average variance = "<<data.num_actions / data.epsilon<<endl;
		uint32_t argmin = find_min(data.cumulative_costs);
		cout<<"last lambda chosen = "<<data.lambdas[argmin]<<" among lambdas ranging from "<<data.lambdas[0]<<" to "<<data.lambdas[data.choices_lambda-1]<<endl;

		for (size_t a = 0; a < data.adf_data.num_actions; ++a)
		{
			COST_SENSITIVE::cs_label.delete_label(&data.csls[a]);
		}
		free(data.csls);
		free(data.cbls);

    for (size_t a = 0; a < data.adf_data.num_actions; ++a)
      {
        data.adf_data.ecs[a]->pred.a_s.delete_v();
        VW::dealloc_example(CB::cb_label.delete_label, *data.adf_data.ecs[a]);
        free_it(data.adf_data.ecs[a]);
      }
    data.adf_data.ecs.~vector<example*>();

		data.lambdas.~vector<float>();
		data.cumulative_costs.~vector<float>();

		data.a_s_adf.delete_v();
		for (size_t i = 0; i < data.ws_vali.size(); ++i)
		{
			if (data.use_cs)
				VW::dealloc_example(COST_SENSITIVE::cs_label.delete_label, *data.ws_vali[i]);
			else
				VW::dealloc_example(MULTICLASS::mc_label.delete_label, *data.ws_vali[i]);
			free(data.ws_vali[i]);
		}
		data.ws_vali.~vector<example*>();
  }
}

void copy_example_to_adf(cbify& data, example& ec)
{
  auto& adf_data = data.adf_data;
  const uint64_t ss = data.all->weights.stride_shift();
  const uint64_t mask = data.all->weights.mask();

  for (size_t a = 0; a < adf_data.num_actions; ++a)
  {
    auto& eca = *adf_data.ecs[a];
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
	// The lambdas are arranged in ascending order
	vector<float>& lambdas = data.lambdas;
	for (uint32_t i = 0; i<data.choices_lambda; i++)
		lambdas.push_back(0.f);

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
		lambdas[i-1] = lambdas[i] / 2.0;

	for (uint32_t i = mid+1; i < data.choices_lambda; i++)
		lambdas[i] = 1 - (1-lambdas[i-1]) / 2.0;

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
	float total_train_size = ws_train_size + inter_train_size;
	float total_weight = (1-data.lambdas[i]) * ws_train_size + data.lambdas[i] * inter_train_size;

	//cout<<i<<" "<<data.lambdas[i]<<endl;
	//cout<<total_weight<<endl;

	if (data.wt_scheme == INSTANCE_WT)
	{
		if (ec_type == WARM_START)
			weight_multiplier = (1-data.lambdas[i]) * total_train_size / (total_weight + FLT_MIN);
		else
			weight_multiplier = data.lambdas[i] * total_train_size / (total_weight + FLT_MIN);
	}
	else
	{
		if (ec_type == WARM_START)
			weight_multiplier = (1-data.lambdas[i]) * total_train_size / ws_train_size;
		else
			weight_multiplier = data.lambdas[i] * total_train_size / inter_train_size;
	}

	//cout<<"weight multiplier: "<<weight_multiplier<<endl;

	return weight_multiplier;
}


template <bool is_learn, bool use_cs>
void predict_or_learn(cbify& data, single_learner& base, example& ec)
{
  //Store the multiclass or cost-sensitive input label
  MULTICLASS::label_t ld;
  COST_SENSITIVE::label csl;
  if (use_cs)
    csl = ec.l.cs;
  else
    ld = ec.l.multi;

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
  if (use_cs)
    cl.cost = loss_cs(data, csl.costs, cl.action);
  else
    cl.cost = loss(data, ld.label, cl.action);

  //Create a new cb label
  data.cb_label.costs.push_back(cl);
  ec.l.cb = data.cb_label;
  base.learn(ec);
  data.a_s.clear();
  data.a_s = ec.pred.a_s;
  if (use_cs)
    ec.l.cs = csl;
  else
    ec.l.multi = ld;
  ec.pred.multiclass = cl.action;
}

uint32_t predict_sublearner_adf(cbify& data, multi_learner& base, example& ec, uint32_t i)
{
	//cout<<"predict using sublearner "<< i <<endl;
	copy_example_to_adf(data, ec);
	//uint32_t offset = data.adf_data.ecs[0]->ft_offset;
	//multiline_learn_or_predict<false>(base, data.adf_data.ecs, offset, i);
	base.predict(data.adf_data.ecs, i);
	//cout<<"greedy label = " << data.adf_data.ecs[0]->pred.a_s[0].action+1 << endl;
	return data.adf_data.ecs[0]->pred.a_s[0].action+1;
}

void accumu_costs_iv_adf(cbify& data, multi_learner& base, example& ec)
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

template<bool use_cs>
void accumu_costs_wsv_adf(cbify& data, multi_learner& base)
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
		//cout<<"validation at iteration "<<data.inter_iter<<endl;
		//cout<<"validation example range: "<< lb << " to " << ub << endl;
		//cout<<"updating validation error on supervised data: " << data.bandit_iter / data.warm_start_period << endl;
		for (uint32_t i = 0; i < data.choices_lambda; i++)
		{
			for (uint32_t j = lb; j < ub; j++)
			{
				example* ec_vali = data.ws_vali[j];
				uint32_t pred_label = predict_sublearner_adf(data, base, *ec_vali, i);

				if (use_cs)
					data.cumulative_costs[i] += loss_cs(data, ec_vali->l.cs.costs, pred_label);
				else
					data.cumulative_costs[i] += loss(data, ec_vali->l.multi.label, pred_label);

				//cout<<ec_vali.l.multi.label<<" "<<pred_label<<endl;
			}
			//cout<<data.cumulative_costs[i]<<endl;
		}
	}
}

template<bool use_cs>
void add_to_vali(cbify& data, example& ec)
{
	//TODO: set the first parameter properly
	example* ec_copy = VW::alloc_examples(sizeof(polylabel), 1);

	if (use_cs)
		VW::copy_example_data(false, ec_copy, &ec, 0, COST_SENSITIVE::cs_label.copy_label);
	else
		VW::copy_example_data(false, ec_copy, &ec, 0, MULTICLASS::mc_label.copy_label);

	data.ws_vali.push_back(ec_copy);
}

uint32_t predict_sup_adf(cbify& data, multi_learner& base, example& ec)
{
	uint32_t argmin = find_min(data.cumulative_costs);
	return predict_sublearner_adf(data, base, ec, argmin);
}

template<bool use_cs>
void learn_sup_adf(cbify& data, multi_learner& base, example& ec, int ec_type)
{
	copy_example_to_adf(data, ec);
	//generate cost-sensitive label (for CSOAA's temporary use)
	auto& csls = data.csls;
	auto& cbls = data.cbls;
	for (size_t a = 0; a < data.adf_data.num_actions; ++a)
	{
		csls[a].costs[0].class_index = a+1;
		if (use_cs)
			csls[a].costs[0].x = loss_cs(data, ec.l.cs.costs, a+1);
 		else
			csls[a].costs[0].x = loss(data, ec.l.multi.label, a+1);
	}
	for (size_t a = 0; a < data.adf_data.num_actions; ++a)
	{
		cbls[a] = data.adf_data.ecs[a]->l.cb;
		data.adf_data.ecs[a]->l.cs = csls[a];
		//cout<<ecs[a].l.cs.costs.size()<<endl;
	}

	vector<float> old_weights;
	for (size_t a = 0; a < data.adf_data.num_actions; ++a)
		old_weights.push_back(data.adf_data.ecs[a]->weight);

	for (uint32_t i = 0; i < data.choices_lambda; i++)
	{
		float weight_multiplier = compute_weight_multiplier(data, i, ec_type);
		//cout<<"weight multiplier in sup = "<<weight_multiplier<<endl;

		for (size_t a = 0; a < data.adf_data.num_actions; ++a)
			data.adf_data.ecs[a]->weight = old_weights[a] * weight_multiplier;
		multi_learner* cs_learner = as_multiline(data.all->cost_sensitive);
		cs_learner->learn(data.adf_data.ecs, i);

		//cout<<"cost-sensitive increment = "<<cs_learner->increment<<endl;
	}
	//Seems like we don't need to set the weights back as this example will be
	//discarded anyway
	for (size_t a = 0; a < data.adf_data.num_actions; ++a)
		data.adf_data.ecs[a]->weight = old_weights[a];

	for (size_t a = 0; a < data.adf_data.num_actions; ++a)
		data.adf_data.ecs[a]->l.cb = cbls[a];
}

template<bool use_cs>
void predict_or_learn_sup_adf(cbify& data, multi_learner& base, example& ec, int ec_type)
{
	uint32_t action = predict_sup_adf(data, base, ec);

	if (ind_update(data, ec_type))
		learn_sup_adf<use_cs>(data, base, ec, ec_type);

	ec.pred.multiclass = action;
}

uint32_t predict_bandit_adf(cbify& data, multi_learner& base, example& ec)
{
	uint32_t argmin = find_min(data.cumulative_costs);

  copy_example_to_adf(data, ec);
	base.predict(data.adf_data.ecs, argmin);

	auto& out_ec = *data.adf_data.ecs[0];
  uint32_t chosen_action;
  if (sample_after_normalizing(data.app_seed + data.example_counter++, begin_scores(out_ec.pred.a_s), end_scores(out_ec.pred.a_s), chosen_action))
    THROW("Failed to sample from pdf");

	//cout<<"predict using sublearner "<< argmin <<endl;
	//cout<<"greedy label = " << data.adf_data.ecs[0]->pred.a_s[0].action+1 << endl;
	//cout<<"chosen action = " << chosen_action << endl;

	auto& a_s = data.a_s_adf;
	copy_array<action_score>(a_s, out_ec.pred.a_s);

	return chosen_action;
}

void learn_bandit_adf(cbify& data, multi_learner& base, example& ec, int ec_type)
{
	copy_example_to_adf(data, ec);

  // add cb label to chosen action
	auto& cl = data.cl_adf;
  auto& lab = data.adf_data.ecs[cl.action - 1]->l.cb;
  lab.costs.push_back(cl);

	vector<float> old_weights;
	for (size_t a = 0; a < data.adf_data.num_actions; ++a)
		old_weights.push_back(data.adf_data.ecs[a]->weight);

	for (uint32_t i = 0; i < data.choices_lambda; i++)
	{
		float weight_multiplier = compute_weight_multiplier(data, i, ec_type);

		//cout<<"learn in sublearner "<< i <<" with weight multiplier "<<weight_multiplier<<endl;
	  for (size_t a = 0; a < data.adf_data.num_actions; ++a)
			data.adf_data.ecs[a]->weight = old_weights[a] * weight_multiplier;
	  base.learn(data.adf_data.ecs, i);

		//cout<<"cb-explore increment = "<<base.increment<<endl;
		//uint32_t offset = data.adf_data.ecs[0]->ft_offset;
		//multiline_learn_or_predict<true>(base, data.adf_data.ecs, offset, i);
	}

	for (size_t a = 0; a < data.adf_data.num_actions; ++a)
		data.adf_data.ecs[a]->weight = old_weights[a];
}

template<bool use_cs>
void predict_or_learn_bandit_adf(cbify& data, multi_learner& base, example& ec, int ec_type)
{
	uint32_t chosen_action = predict_bandit_adf(data, base, ec);

	auto& cl = data.cl_adf;
	auto& a_s = data.a_s_adf;
	cl.action = a_s[chosen_action].action + 1;
	cl.probability = a_s[chosen_action].score;

	//cout<<cl.action<<" "<<cl.probability<<" "<<ec.l.multi.label<<endl;

	if(!cl.action)
		THROW("No action with non-zero probability found!");

	if (use_cs)
		cl.cost = loss_cs(data, ec.l.cs.costs, cl.action);
	else
		cl.cost = loss(data, ec.l.multi.label, cl.action);

	if (ec_type == INTERACTION && data.vali_method == INTER_VALI)
		accumu_costs_iv_adf(data, base, ec);

	//cout<<cl.action<<" "<<cl.probability<<endl;

	if (ind_update(data, ec_type))
		learn_bandit_adf(data, base, ec, ec_type);

	if (ec_type == INTERACTION && (data.vali_method == WS_VALI_SPLIT || data.vali_method == WS_VALI_NOSPLIT))
		accumu_costs_wsv_adf<use_cs>(data, base);

	ec.pred.multiclass = cl.action;
}

void accumu_var_adf(cbify& data, multi_learner& base, example& ec)
{
	size_t pred_best_approx = predict_sup_adf(data, base, ec);
	float temp_var;

	for (size_t a = 0; a < data.adf_data.num_actions; ++a)
		if (pred_best_approx == data.a_s_adf[a].action + 1)
			temp_var = 1.0 / data.a_s_adf[a].score;

	data.cumu_var += temp_var;

	//cout<<"variance at bandit round "<< data.inter_iter << " = " << temp_var << endl;
	//cout<<pred_best_approx<<" "<<data.a_s_adf[0].action+1<<endl;
}

template <bool is_learn, bool use_cs>
void predict_or_learn_adf(cbify& data, multi_learner& base, example& ec)
{
	// Corrupt labels (only corrupting multiclass labels as of now)

	if (use_cs)
		data.cs_label = ec.l.cs;
	else
	{
		data.mc_label = ec.l.multi;
		/*if (data.ws_iter < data.ws_period)
			ec.l.multi.label = corrupt_action(data, data.mc_label.label, WARM_START);
		else if (data.inter_iter < data.inter_period)
			ec.l.multi.label = corrupt_action(data, data.mc_label.label, INTERACTION);
		*/
	}

	// Warm start phase
	if (data.ws_iter < data.ws_period)
	{
		if (data.ws_iter < data.ws_train_size)
		{
			if (data.ws_type == SUPERVISED_WS)
				predict_or_learn_sup_adf<use_cs>(data, base, ec, WARM_START);
			else if (data.ws_type == BANDIT_WS)
				predict_or_learn_bandit_adf<use_cs>(data, base, ec, WARM_START);
		}
		else
			add_to_vali<use_cs>(data, ec);
		ec.weight = 0;
		data.ws_iter++;
	}
	// Interaction phase
	else if (data.inter_iter < data.inter_period)
	{
		predict_or_learn_bandit_adf<use_cs>(data, base, ec, INTERACTION);
		accumu_var_adf(data, base, ec);
		data.a_s_adf.clear();
		data.inter_iter++;
	}
	// Skipping the rest of the examples
	else
		ec.weight = 0;

	// Store the original labels back
	if (use_cs)
		ec.l.cs = data.cs_label;
	else
		ec.l.multi = data.mc_label;

}

void init_adf_data(cbify& data, const size_t num_actions)
{
  auto& adf_data = data.adf_data;
  adf_data.num_actions = num_actions;

  adf_data.ecs.resize(num_actions);
  for (size_t a=0; a < num_actions; ++a)
  {
    adf_data.ecs[a] = VW::alloc_examples(CB::cb_label.label_size, 1);
    auto& lab = adf_data.ecs[a]->l.cb;
    CB::cb_label.default_label(&lab);
  }

	// The rest of the initialization is for warm start CB
	data.csls = calloc_or_throw<COST_SENSITIVE::label>(num_actions);
	for (uint32_t a=0; a < num_actions; ++a)
	{
		COST_SENSITIVE::cs_label.default_label(&data.csls[a]);
		data.csls[a].costs.push_back({0, a+1, 0, 0});
	}
	data.cbls = calloc_or_throw<CB::label>(num_actions);

	if (data.vali_method == WS_VALI_SPLIT || data.vali_method == WS_VALI_NOSPLIT)
	{
		data.ws_train_size = ceil(data.ws_period / 2.0);
		data.ws_vali_size = data.ws_period - data.ws_train_size;
	}
	else
	{
		data.ws_train_size = data.ws_period;
		data.ws_vali_size = 0;
	}
	data.ws_iter = 0;
	data.inter_iter = 0;

	setup_lambdas(data);
	for (uint32_t i = 0; i < data.choices_lambda; i++)
		data.cumulative_costs.push_back(0.f);
	data.cumu_var = 0.f;
}

base_learner* cbify_setup(arguments& arg)
{
  uint32_t num_actions=0;
  auto data = scoped_calloc_or_throw<cbify>();
  bool use_cs;

  if (arg.new_options("Make Multiclass into Contextual Bandit")
      .critical("cbify", num_actions, "Convert multiclass on <k> classes into a contextual bandit problem")
      (use_cs, "cbify_cs", "consume cost-sensitive classification examples instead of multiclass")
      ("loss0", data->loss0, 0.f, "loss for correct label")
      ("loss1", data->loss1, 1.f, "loss for incorrect label")
			("warm_start", data->ws_period, 0U, "number of training examples for warm start phase")
			("interaction", data->inter_period, UINT32_MAX, "number of examples for the interactive contextual bandit learning phase")
			("warm_start_update", data->upd_ws, true, "indicator of warm start updates")
			("interaction_update", data->upd_inter, true, "indicator of interaction updates")
			("corrupt_type_warm_start", data->cor_type_ws, UAR, "type of label corruption in the warm start phase (1: uniformly at random, 2: circular, 3: replacing with overwriting label)")
			("corrupt_prob_warm_start", data->cor_prob_ws, 0.f, "probability of label corruption in the warm start phase")
			("corrupt_type_interaction", data->cor_type_inter, UAR, "type of label corruption in the interaction phase (1: uniformly at random, 2: circular, 3: replacing with overwriting label)")
			("corrupt_prob_interaction", data->cor_prob_inter, 0.f, "probability of label corruption in the interaction phase")
		  ("choices_lambda", data->choices_lambda, 1U, "the number of candidate lambdas to aggregate (lambda is the importance weight parameter between the two sources) ")
			("lambda_scheme", data->lambda_scheme, ABS_CENTRAL, "The scheme for generating candidate lambda set (1: center lambda=0.5, 2: center lambda=0.5, min lambda=0, max lambda=1, 3: center lambda=epsilon/(#actions+epsilon), 4: center lambda=epsilon/(#actions+epsilon), min lambda=0, max lambda=1); the rest of candidate lambda values are generated using a doubling scheme")
			("weighting_scheme", data->wt_scheme, INSTANCE_WT, "weighting scheme (1: per instance weighting, where for every lambda, each contextual bandit example have weight lambda/(1-lambda) times that of each warm start example, 2: per dataset weighting, where for every lambda, the contextual bandit dataset has total weight lambda/(1-lambda) times that of the warm start dataset)")
			("validation_method", data->vali_method, INTER_VALI, "lambda selection criterion (1: using contextual bandit examples with progressive validation, 2: using warm start examples, with fresh validation examples at each epoch, 3: using warm start examples, with a single validation set throughout)")
			("overwrite_label", data->overwrite_label, 1U, "the label used by type 3 corruptions (overwriting)")
			("warm_start_type", data->ws_type, SUPERVISED_WS, "update method of utilizing warm start examples (1: using supervised updates, 2: using contextual bandit updates)").missing())
    return nullptr;

  data->use_adf = count(arg.args.begin(), arg.args.end(),"--cb_explore_adf") > 0;
  data->app_seed = uniform_hash("vw", 2, 0);
  data->a_s = v_init<action_score>();
  data->all = arg.all;

	data->num_actions = num_actions;
	data->use_cs = use_cs;

  if (data->use_adf)
    init_adf_data(*data.get(), num_actions);

  if (count(arg.args.begin(), arg.args.end(),"--cb_explore") == 0 && !data->use_adf)
  {
    arg.args.push_back("--cb_explore");
    stringstream ss;
    ss << num_actions;
    arg.args.push_back(ss.str());
  }
  if (data->use_adf)
    {
      arg.args.push_back("--cb_min_cost");
      arg.args.push_back(to_string(data->loss0));
      arg.args.push_back("--cb_max_cost");
      arg.args.push_back(to_string(data->loss1));
    }
  if (count(arg.args.begin(), arg.args.end(), "--baseline"))
  {
    arg.args.push_back("--lr_multiplier");
    stringstream ss;
    ss << max<float>(abs(data->loss0), abs(data->loss1)) / (data->loss1 - data->loss0);
    arg.args.push_back(ss.str());
  }

  learner<cbify,example>* l;

  if (data->use_adf)
  {
    multi_learner* base = as_multiline(setup_base(arg));
		// Note: the current version of warm start CB can only support epsilon greedy exploration
		// algorithm - we need to wait for the default epsilon value to be passed from cb_explore
		// is there is one
		//cout<<"count: "<<arg.vm.count("epsilon") <<endl;
		data->epsilon = arg.vm.count("epsilon") > 0 ? arg.vm["epsilon"].as<float>() : 0.0f;

    if (use_cs)
      l = &init_cost_sensitive_learner(data, base, predict_or_learn_adf<true, true>, predict_or_learn_adf<false, true>, arg.all->p, data->choices_lambda);
    else
      l = &init_multiclass_learner(data, base, predict_or_learn_adf<true, false>, predict_or_learn_adf<false, false>, arg.all->p, data->choices_lambda);

		//cout<<"cbify increment = "<<l->increment<<endl;
  }
  else
  {
    single_learner* base = as_singleline(setup_base(arg));
    if (use_cs)
      l = &init_cost_sensitive_learner(data, base, predict_or_learn<true, true>, predict_or_learn<false, true>, arg.all->p, 1);
    else
      l = &init_multiclass_learner(data, base, predict_or_learn<true, false>, predict_or_learn<false, false>, arg.all->p, 1);
  }
  l->set_finish(finish);
  arg.all->delete_prediction = nullptr;

  return make_base(*l);
}

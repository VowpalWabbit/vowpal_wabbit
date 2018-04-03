#include <float.h>
#include "reductions.h"
#include "cb_algs.h"
#include "rand48.h"
#include "bs.h"
#include "../explore/cpp/MWTExplorer.h"
#include "vw.h"
#include <random>

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
	bool warm_start;
	float* old_weights;

	float corrupt_prob_supervised;
	float corrupt_prob_bandit;
	size_t corrupt_type_supervised;
	size_t corrupt_type_bandit;
	size_t validation_method;
	size_t bandit_iter;
	size_t warm_start_iter;
	size_t weighting_scheme;
	v_array<example> supervised_validation;

};

float rand_zeroone()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(0.0, 1.0);
	return dis(gen);
	//return 0.5;
}


size_t generate_uar_action(size_t num_actions)
{
	float rand = rand_zeroone();
	//cout<<rand<<endl;

	for (size_t i = 1; i <= num_actions; i++)
	{
		if (rand <= float(i) / num_actions)
			return i;
	}	
	return num_actions;

}

size_t corrupt_action(size_t action, size_t num_actions, float label_corrupt, size_t type)
{
	float rand = rand_zeroone();
	if (rand < label_corrupt)
	{
		if (type == 1)
			return generate_uar_action(num_actions);
		else 
			return (action % num_actions) + 1;
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

void finish(cbify& data)
{
  CB::cb_label.delete_label(&data.cb_label);
  //data.probs.delete_v();
  delete_it(data.scorer);
  delete_it(data.generic_explorer);
  delete_it(data.mwt_explorer);
  delete_it(data.recorder);
  data.a_s.delete_v();
	data.lambdas.delete_v();
	data.cumulative_costs.delete_v();


  if (data.use_adf)
  {
	  for (size_t a = 0; a < data.adf_data.num_actions; ++a)
	  {
			VW::dealloc_example(CB::cb_label.delete_label, data.adf_data.ecs[a]);
			data.adf_data.ecs[a].pred.a_s.delete_v();
	  }
	  VW::dealloc_example(CB::cb_label.delete_label, *data.adf_data.empty_example);
		data.adf_data.empty_example->pred.a_s.delete_v();

    free(data.adf_data.ecs);
    free(data.adf_data.empty_example);

		for (size_t a = 0; a < data.adf_data.num_actions; ++a)
			data.csls[a].costs.delete_v();

		data.csl_empty->costs.delete_v();

		free(data.csl_empty);
		free(data.cbl_empty);

		free(data.old_weights);
		free(data.cbls);

  }
	free(data.csls);


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
	//cout<<"argmin = "<<argmin<<endl;

	return argmin;
}

uint32_t predict_sublearner(cbify& data, base_learner& base, uint32_t i)
{
	example* ecs = data.adf_data.ecs;
	example* empty = data.adf_data.empty_example;


	for (size_t a = 0; a < data.adf_data.num_actions; ++a)
	{
	  base.predict(ecs[a], i);
		//data.all->cost_sensitive->predict(ecs[a], argmin);
	}
	base.predict(*empty, i);
	//data.all->cost_sensitive->predict(*empty, argmin);


	//float best_score;
	//for (size_t a = 0; a < data.adf_data.num_actions; ++a)
	//{
	//	if ( (a == 0) || (ecs[a].partial_prediction < best_score) )
	//	{
	//		best_action = a + 1;
	//		best_score = ecs[a].partial_prediction;
	//	}
	//}
	//best_action_dir = ecs[0].pred.a_s[0].action+1;
	//assert(best_action == best_action_dir);

	return ecs[0].pred.a_s[0].action+1;

}



void accumulate_costs_ips(cbify& data, example& ec, CB::cb_class& cl)
{
	// validation using bandit data
	if (data.validation_method == 1)
	{
		//IPS for approximating the cumulative costs for all lambdas
		for (uint32_t i = 0; i < data.choices_lambda; i++)
		{
			data.all->cost_sensitive->predict(ec, i);
			if (ec.pred.multiclass == cl.action)
				data.cumulative_costs[i] += cl.cost / cl.probability;
			//cout<<data.cumulative_costs[i]<<endl;
		}
		//cout<<endl;
	}
	else //validation using supervised data (their labels are already set to cost-sensitive labels)
	{
		//only update cumulative costs every warm_start_period iterations
		if (data.bandit_iter % data.warm_start_period == 0)
		{
			for (uint32_t i = 0; i < data.choices_lambda; i++)
				data.cumulative_costs[i] = 0;

			//cout<<"updating validation error on supervised data: " << data.bandit_iter / data.warm_start_period << endl;
			for (uint32_t i = 0; i < data.choices_lambda; i++)
			{
				//go over the supervised validation set
				for (uint32_t j = 0; j < data.supervised_validation.size(); j++)
				{
					example& ec_valid = data.supervised_validation[j];
					data.all->cost_sensitive->predict(ec_valid, i);

					//cout<<ec_valid.pred.multiclass<<endl;
					//for (uint32_t a = 0; a < data.num_actions; a++)
					//	cout<<ec_valid.l.cs.costs[a].class_index<<" "<<ec_valid.l.cs.costs[a].x<<endl;
					//cout<<endl;

					for (uint32_t a = 0; a < data.num_actions; a++)
						if (ec_valid.pred.multiclass == ec_valid.l.cs.costs[a].class_index)
							data.cumulative_costs[i] += ec_valid.l.cs.costs[a].x;
				}
				//cout<<data.cumulative_costs[i]<<endl;
			}
		}
	}
}

void accumulate_costs_ips_adf(cbify& data, example& ec, CB::cb_class& cl, base_learner& base)
{
	
	if (data.validation_method == 1)
	{
		uint32_t best_action;

		//IPS for approximating the cumulative costs for all lambdas
		for (uint32_t i = 0; i < data.choices_lambda; i++)
		{
			best_action = predict_sublearner(data, base, i);

			if (best_action == cl.action)
				data.cumulative_costs[i] += cl.cost / cl.probability;

			//cout<<data.cumulative_costs[i]<<endl;
		}
		//cout<<endl;
	}
	else
	{
		//only update cumulative costs every warm_start_period iterations
		if (data.bandit_iter % data.warm_start_period == 0)
		{
			for (uint32_t i = 0; i < data.choices_lambda; i++)
				data.cumulative_costs[i] = 0;

			//cout<<"updating validation error on supervised data: " << data.bandit_iter / data.warm_start_period << endl;
			for (uint32_t i = 0; i < data.choices_lambda; i++)
			{
				for (uint32_t j = 0; j < data.supervised_validation.size(); j++)
				{
					example& ec_valid = data.supervised_validation[j];
			  	copy_example_to_adf(data, ec_valid);
					uint32_t pred_label = predict_sublearner(data, base, i);
					data.cumulative_costs[i] += loss(data, ec_valid.l.multi.label, pred_label);
					//cout<<ec_valid.l.multi.label<<" "<<pred_label<<endl;
				}
				//cout<<data.cumulative_costs[i]<<endl;
			}
			
		}
		
	}

}



template <bool is_learn>
void predict_or_learn(cbify& data, base_learner& base, example& ec)
{
	float old_weight;
	uint32_t argmin;

	argmin = find_min(data.cumulative_costs);

	//Store the multiclass input label
	MULTICLASS::label_t ld = ec.l.multi;

	//cout<<ld.label<<endl;

	if (data.warm_start_iter < data.warm_start_period) // Call the cost-sensitive learner directly
	{
		/*
		//generate cost-sensitive label
		COST_SENSITIVE::label& csl = *data.csls;
    csl.costs.resize(data.num_actions);
    csl.costs.end() = csl.costs.begin()+data.num_actions;
		for (uint32_t j = 0; j < data.num_actions; j++)
		{
			csl.costs[j].class_index = j+1;
			csl.costs[j].x = loss(data, ld.label, j+1);
		}

		ec.l.cs = csl;

		//predict
		data.all->cost_sensitive->predict(ec, argmin);
		*/

		//first, corrupt fully supervised example ec's label here
		size_t corrupted_label = corrupt_action(ld.label, data.num_actions, data.corrupt_prob_supervised, data.corrupt_type_supervised);
		//use this for now; I am not sure if v_array is the same as STL's array where elements are copied when brought in
		ld.label = corrupted_label;

		//generate cost-sensitive label
		//COST_SENSITIVE::label& csl = *data.csls;
		COST_SENSITIVE::label* cslp = calloc_or_throw<COST_SENSITIVE::label>(1);
		COST_SENSITIVE::label csl = *cslp;
    csl.costs.resize(data.num_actions);
    csl.costs.end() = csl.costs.begin()+data.num_actions;
		for (uint32_t j = 0; j < data.num_actions; j++)
		{
			csl.costs[j].class_index = j+1;
			csl.costs[j].x = loss(data, corrupted_label, j+1);
		}

		ec.l.cs = csl;	

		//predict (for vw's internal reason, this step has to be put after ec's cs label is created)
		data.all->cost_sensitive->predict(ec, argmin);

		if (data.ind_supervised)
		{
			
			for (uint32_t i = 0; i < data.choices_lambda; i++)
			{
				ec.weight = 1;
				data.all->cost_sensitive->learn(ec, i);
			}
		}
		
		//ec.l.multi = ld;
		ec.weight = 0;

		// This is purely a hack here - need to clean up; I also did not deallocate the label and the copied example in finish()
		example* ecp = calloc_or_throw<example>(1);		
		VW::copy_example_data(false, ecp, &ec);
		ecp->l.cs = csl;

		// I am not sure if written this way, ec will be deleted in some other stages and causes error
		if (data.validation_method == 2)
			data.supervised_validation.push_back(*ecp);

		data.warm_start_iter++;
	}
	else if (data.bandit_iter < data.bandit_period)//Call the cb_explore algorithm. It returns a vector of probabilities for each action
	{
		data.cb_label.costs.erase();
		ec.l.cb = data.cb_label;
		ec.pred.a_s = data.a_s;

		base.predict(ec, argmin);
		auto old_pred = ec.pred;

		uint32_t action = data.mwt_explorer->Choose_Action(*data.generic_explorer, StringUtils::to_string(data.example_counter++), ec);

		CB::cb_class cl;
		cl.action = action;
		cl.probability = ec.pred.a_s[action-1].score;

		if(!cl.action)
		  THROW("No action with non-zero probability found!");

		size_t corrupted_label = corrupt_action(ld.label, data.num_actions, data.corrupt_prob_bandit, data.corrupt_type_bandit);
		cl.cost = loss(data, corrupted_label, cl.action);

		// accumulate the cumulative costs of lambdas
		accumulate_costs_ips(data, ec, cl);

		//Create a new cb label
		data.cb_label.costs.push_back(cl);
		ec.l.cb = data.cb_label;

		ec.pred = old_pred;
		old_weight = ec.weight;

		if (data.ind_bandit)
		{
			for (uint32_t i = 0; i < data.choices_lambda; i++)
			{
				if (data.weighting_scheme == 1)
					ec.weight = old_weight * data.lambdas[i] / (1-data.lambdas[i]);
				else
					ec.weight = old_weight * data.lambdas[i] / (1-data.lambdas[i]) * data.warm_start_period / ( (data.bandit_iter+1) * (data.bandit_iter+2) );

				base.learn(ec, i);
			}
		}
		data.a_s.erase();
		data.a_s = ec.pred.a_s;
		ec.l.multi = ld;
	  ec.pred.multiclass = action;
		ec.weight = old_weight;

		data.bandit_iter++;
	}
	else
	{
		//skipping
		//base.predict(ec, argmin);
		ec.pred.multiclass = 0;
		ec.weight = 0;

	}
}


template <bool is_learn>
void predict_or_learn_adf(cbify& data, base_learner& base, example& ec)
{
	uint32_t argmin;
	uint32_t best_action;
	example* ecs = data.adf_data.ecs;
	example* empty_example = data.adf_data.empty_example;

	argmin = find_min(data.cumulative_costs);

  //Store the multiclass input label
  MULTICLASS::label_t ld = ec.l.multi;

  copy_example_to_adf(data, ec);

	if (data.warm_start_iter < data.warm_start_period) // Call the cost-sensitive learner directly
	{

		best_action = predict_sublearner(data, base, argmin);

		//data.all->cost_sensitive->predict(ec,argmin);

		//generate cost-sensitive label
		// ecs[a].weight *= 1;
		//				cout << "size cbify = " << ecs[a].l.cs.costs.size() << endl;

		COST_SENSITIVE::label* csls = data.csls;
		COST_SENSITIVE::label* csl_empty = data.csl_empty;
		CB::label* cbls = data.cbls;
		CB::label* cbl_empty = data.cbl_empty;

		size_t corrupted_label = corrupt_action(ld.label, data.num_actions, data.corrupt_prob_supervised, data.corrupt_type_supervised);

		if (data.ind_supervised)
		{
			for (uint32_t i = 0; i < data.choices_lambda; i++)
			{
				for (size_t a = 0; a < data.adf_data.num_actions; ++a)
				{
					csls[a].costs[0].class_index = a+1;
					csls[a].costs[0].x = loss(data, corrupted_label, a+1);

					cbls[a] = ecs[a].l.cb;
					ecs[a].l.cs = csls[a];
					data.all->cost_sensitive->learn(ecs[a],i);
				}
				*cbl_empty = empty_example->l.cb;
				empty_example->l.cs = *csl_empty;
				data.all->cost_sensitive->learn(*empty_example,i);

				for (size_t a = 0; a < data.adf_data.num_actions; ++a)
					ecs[a].l.cb = cbls[a];
				empty_example->l.cb = *cbl_empty;
			}
		}
		ec.pred.multiclass = best_action;
		ec.l.multi = ld;
		ec.weight = 0;

		//a hack here - allocated memories not deleted
		example* ecp = calloc_or_throw<example>(1);	
		VW::copy_example_data(false, ecp, &ec);
		ecp->l.multi.label = corrupted_label;
		ecp->l.multi.weight = 1.0;

		if (data.validation_method == 2)
			data.supervised_validation.push_back(*ecp);

		data.warm_start_iter++;	

	}
	else if (data.bandit_iter < data.bandit_period) // call the bandit learner
	{

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

		CB::cb_class cl;
		cl.action = out_ec.pred.a_s[idx].action + 1;
		cl.probability = out_ec.pred.a_s[idx].score;

		if(!cl.action)
		  THROW("No action with non-zero probability found!");

		size_t corrupted_label = corrupt_action(ld.label, data.num_actions, data.corrupt_prob_bandit, data.corrupt_type_bandit);
		cl.cost = loss(data, corrupted_label, cl.action);

		// accumulate the cumulative costs of lambdas
		accumulate_costs_ips_adf(data, ec, cl, base);

		// add cb label to chosen action
		auto& lab = data.adf_data.ecs[cl.action - 1].l.cb;
		lab.costs.push_back(cl);


		if (data.ind_bandit)
		{
			for (uint32_t i = 0; i < data.choices_lambda; i++)
			{
				for (size_t a = 0; a < data.adf_data.num_actions; ++a)
				{
					data.old_weights[a] = ecs[a].weight;

					if (data.weighting_scheme == 1)
						ecs[a].weight *= data.lambdas[i] / (1-data.lambdas[i]);
					else
						ecs[a].weight *= data.lambdas[i] / (1-data.lambdas[i]) * data.warm_start_period / ( (data.bandit_iter+1) * (data.bandit_iter+2) );					

					base.learn(ecs[a], i);
				}
				base.learn(*empty_example, i);

				for (size_t a = 0; a < data.adf_data.num_actions; ++a)
					ecs[a].weight = data.old_weights[a];
			}
		}

		ec.pred.multiclass = cl.action;
			
		data.bandit_iter++;
	}
	else
	{
		ec.pred.multiclass = 0;
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
	adf_data.empty_example->pred.a_s = v_init<action_score>();


	data.csls = calloc_or_throw<COST_SENSITIVE::label>(num_actions);
	data.csl_empty = calloc_or_throw<COST_SENSITIVE::label>(1);
	data.cbls = calloc_or_throw<CB::label>(num_actions);
	data.cbl_empty = calloc_or_throw<CB::label>(1);


	data.old_weights = calloc_or_throw<float>(num_actions);

	data.csl_empty->costs.push_back({0, 0, 0, 0});
	data.csl_empty->costs[0].class_index = 0;
	data.csl_empty->costs[0].x = FLT_MAX;

	for (size_t a = 0; a < num_actions; ++a)
	{
		data.csls[a].costs.push_back({0, 0, 0, 0});
	}

}

void generate_lambdas(v_array<float>& lambdas, size_t lambda_size)
{
	// The lambdas are in fact arranged in ascending order (the middle lambda is 0.5)

	lambdas = v_init<float>();
	uint32_t mid = lambda_size / 2;
	for (uint32_t i = 0; i < lambda_size; i++)
		lambdas.push_back(0);

	lambdas[mid] = 0.5;
	for (uint32_t i = mid; i > 0; i--)
		lambdas[i-1] = lambdas[i] / 2;

	for (uint32_t i = mid+1; i < lambda_size; i++)
		lambdas[i] = 1 - (1-lambdas[i-1]) / 2;

}

base_learner* cbify_setup(vw& all)
{
  //parse and set arguments
  if (missing_option<size_t, true>(all, "cbify", "Convert multiclass on <k> classes into a contextual bandit problem"))
    return nullptr;
  new_options(all, "CBIFY options")
  ("loss0", po::value<float>(), "loss for correct label")
  ("loss1", po::value<float>(), "loss for incorrect label")
	("warm_start", po::value<size_t>(), "number of training examples for fully-supervised warm start")
	("bandit", po::value<size_t>(), "number of training examples for bandit processing")
  ("choices_lambda", po::value<size_t>(), "numbers of lambdas importance weights to aggregate")
	("no_supervised", "indicator of using supervised only")
	("no_bandit", "indicator of using bandit only")
	("corrupt_prob_supervised", po::value<float>(), "probability of label corruption in the supervised part")
	("corrupt_prob_bandit", po::value<float>(), "probability of label corruption in the bandit part")
	("corrupt_type_supervised", po::value<size_t>(), "type of label corruption in the supervised part (1 is uar, 2 is circular)")
	("corrupt_type_bandit", po::value<size_t>(), "probability of label corruption in the bandit part (1 is uar, 2 is circular)")
	("validation_method", po::value<size_t>(), "lambda selection criterion (1 is using bandit with progressive validation, 2 is using supervised and amortizing)")
	("weighting_scheme", po::value<size_t>(), "weighting scheme (1 is per instance weighting, 2 is per dataset weighting (where we use a diminishing weighting scheme) )");
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
	data.corrupt_type_supervised = vm.count("corrupt_type_supervised") ? vm["corrupt_type_supervised"].as<size_t>() : 1;
	data.corrupt_type_bandit = vm.count("corrupt_type_bandit") ? vm["corrupt_type_bandit"].as<size_t>() : 1;
	data.validation_method = vm.count("validation_method") ? vm["validation_method"].as<size_t>() : 1;
	data.weighting_scheme = vm.count("weighting_scheme") ? vm["weighting_scheme"].as<size_t>() : 1;


	data.bandit_iter = 0;
	data.warm_start_iter = 0;
	

	generate_lambdas(data.lambdas, data.choices_lambda);

	for (size_t i = 0; i < data.choices_lambda; i++)
		data.cumulative_costs.push_back(0.);

	data.num_actions = num_actions;


  if (data.use_adf)
  {
    init_adf_data(data, num_actions);
  }
	else
	{
		data.csls = calloc_or_throw<COST_SENSITIVE::label>(1);
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

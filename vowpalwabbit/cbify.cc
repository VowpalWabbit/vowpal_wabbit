#include <float.h>
#include "reductions.h"
#include "cb_algs.h"
#include "rand48.h"
#include "bs.h"
#include "../explore/cpp/MWTExplorer.h"
#include "vw.h"

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
	v_array<float> cumulative_costs;
	v_array<float> lambdas;
	size_t num_actions;

};

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
  if (data.use_adf)
  {
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
		if (arr[i] < min_val)
		{	
			min_val = arr[i];
			argmin = i;
		}
	}

	return argmin;
}

template <bool is_learn>
void predict_or_learn(cbify& data, base_learner& base, example& ec)
{
	bool is_supervised;

	if (data.warm_start_period > 0)
	{
		is_supervised = true;
		data.warm_start_period--;
	}	
	else
		is_supervised = false;

	uint32_t argmin;
	argmin = find_min(data.cumulative_costs);
	if (argmin != 0)
		cout<<"argmin is not zero"<<endl;

	//Store the multiclass input label
	MULTICLASS::label_t ld = ec.l.multi;

	//cout<<ld.label<<endl;
  
	if (is_supervised)
	{
		//generate cost-sensitive label
		COST_SENSITIVE::label csl;
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
		//uint32_t chosen = ec.pred.multiclass-1;	
		//cout<<ec.pred.multiclass<<endl;


		for (uint32_t i = 0; i < data.choices_lambda; i++)
		{
			ec.weight = 1;
			data.all->cost_sensitive->learn(ec, i);
		}
		ec.l.multi = ld;
	}
	else //Call the cb_explore algorithm. It returns a vector of probabilities for each action
	{
		data.cb_label.costs.erase();
		ec.l.cb = data.cb_label;
		ec.pred.a_s = data.a_s;
		
		base.predict(ec, argmin);
		//base.predict(ec);
		//data.probs = ec.pred.scalars;

		uint32_t action = data.mwt_explorer->Choose_Action(*data.generic_explorer, StringUtils::to_string(data.example_counter++), ec);

		CB::cb_class cl;
		cl.action = action;
		cl.probability = ec.pred.a_s[action-1].score;

		if(!cl.action)
		  THROW("No action with non-zero probability found!");
		cl.cost = loss(data, ld.label, cl.action);

		//Create a new cb label
		data.cb_label.costs.push_back(cl);
		ec.l.cb = data.cb_label;
		//base.learn(ec);
		for (uint32_t i = 0; i < data.choices_lambda; i++)
		{
			ec.weight = data.lambdas[i] / (1-data.lambdas[i]);
			base.learn(ec, i);
			data.cumulative_costs[i] += 0;
		}

		data.a_s.erase();
		data.a_s = ec.pred.a_s;
		ec.l.multi = ld;
		ec.pred.multiclass = action;
	}
}

template <bool is_learn>
void predict_or_learn_adf(cbify& data, base_learner& base, example& ec)
{
  //Store the multiclass input label
  MULTICLASS::label_t ld = ec.l.multi;

  copy_example_to_adf(data, ec);
  for (size_t a = 0; a < data.adf_data.num_actions; ++a)
  {
    base.predict(data.adf_data.ecs[a]);
  }
  base.predict(*data.adf_data.empty_example);
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
  cl.cost = loss(data, ld.label, cl.action);

  // add cb label to chosen action
  auto& lab = data.adf_data.ecs[cl.action - 1].l.cb;
  lab.costs.push_back(cl);

  for (size_t a = 0; a < data.adf_data.num_actions; ++a)
  {
    base.learn(data.adf_data.ecs[a]);
  }
  base.learn(*data.adf_data.empty_example);
  ec.pred.multiclass = cl.action;
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
}

void generate_lambdas(v_array<float>& lambdas, size_t lambda_size)
{
	lambdas = v_init<float>();
	for (uint32_t i = 0; i < lambda_size; i++)
		if (i%2 == 0)
		{
			lambdas.push_back(pow(0.5f, floor(i/2) + 1));
			//cout<<pow(0.5f, floor(i/2) + 1)<<endl;
		}			
		else
		{	
			lambdas.push_back(1 - pow(0.5f, floor(i/2) + 2));
			//cout<<1 - pow(0.5f, floor(i/2) + 2)<<endl;
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
	("warm_start", po::value<size_t>(), "number of training examples for fully-supervised warm start")
  ("choices_lambda", po::value<size_t>(), "numbers of lambdas importance weights to aggregate");
  add_options(all);

  po::variables_map& vm = all.vm;
  uint32_t num_actions = (uint32_t)vm["cbify"].as<size_t>();

  cbify& data = calloc_or_throw<cbify>();
  data.use_adf = count(all.args.begin(), all.args.end(),"--cb_explore_adf") > 0;
  data.loss0 = vm.count("loss0") ? vm["loss0"].as<float>() : 0.f;
  data.loss1 = vm.count("loss1") ? vm["loss1"].as<float>() : 1.f;
  data.recorder = new vw_recorder();
  data.mwt_explorer = new MwtExplorer<example>("vw",*data.recorder);
  data.scorer = new vw_scorer();
  data.a_s = v_init<action_score>();
  //data.probs = v_init<float>();
  data.generic_explorer = new GenericExplorer<example>(*data.scorer, (u32)num_actions);
  data.all = &all;

	cout<<data.warm_start_period<<endl;
	data.warm_start_period = vm.count("warm_start") ? vm["warm_start"].as<size_t>() : 0;
	cout<<data.warm_start_period<<endl;
	data.choices_lambda = vm.count("choices_lambda") ? vm["choices_lambda"].as<size_t>() : 1;

	generate_lambdas(data.lambdas, data.choices_lambda);

	for (size_t i = 0; i < data.choices_lambda; i++)
		data.cumulative_costs.push_back(0.);

	data.num_actions = num_actions;

  if (data.use_adf)
  {
    init_adf_data(data, num_actions);
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

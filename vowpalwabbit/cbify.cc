#include <float.h>
#include "reductions.h"
#include "cb_algs.h"
#include "rand48.h"
#include "bs.h"
#include "vw.h"
#include "hash.h"
#include "explore.h"

#include <vector>

using namespace LEARNER;
using namespace exploration;
using namespace ACTION_SCORE;
using namespace std;

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

void finish(cbify& data)
{
  CB::cb_label.delete_label(&data.cb_label);
  data.a_s.delete_v();
  if (data.use_adf)
  {
    for (size_t a = 0; a < data.adf_data.num_actions; ++a)
      {
        data.adf_data.ecs[a]->pred.a_s.delete_v();
        VW::dealloc_example(CB::cb_label.delete_label, *data.adf_data.ecs[a]);
        free_it(data.adf_data.ecs[a]);
      }
    data.adf_data.ecs.~vector<example*>();
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

template <bool is_learn, bool use_cs>
void predict_or_learn_adf(cbify& data, multi_learner& base, example& ec)
{
  //Store the multiclass or cost-sensitive input label
  MULTICLASS::label_t ld;
  COST_SENSITIVE::label csl;
  if (use_cs)
    csl = ec.l.cs;
  else
    ld = ec.l.multi;

  copy_example_to_adf(data, ec);
  base.predict(data.adf_data.ecs);

  auto& out_ec = *data.adf_data.ecs[0];

  uint32_t chosen_action;
  if (sample_after_normalizing(data.app_seed + data.example_counter++, begin_scores(out_ec.pred.a_s), end_scores(out_ec.pred.a_s), chosen_action))
    THROW("Failed to sample from pdf");

  CB::cb_class cl;
  cl.action = out_ec.pred.a_s[chosen_action].action + 1;
  cl.probability = out_ec.pred.a_s[chosen_action].score;

  if(!cl.action)
    THROW("No action with non-zero probability found!");

  if (use_cs)
    cl.cost = loss_cs(data, csl.costs, cl.action);
  else
    cl.cost = loss(data, ld.label, cl.action);

  // add cb label to chosen action
  auto& lab = data.adf_data.ecs[cl.action - 1]->l.cb;
  lab.costs.push_back(cl);

  base.learn(data.adf_data.ecs);
  ec.pred.multiclass = cl.action;
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
      ("loss1", data->loss1, 1.f, "loss for incorrect label").missing())
    return nullptr;

  data->use_adf = count(arg.args.begin(), arg.args.end(),"--cb_explore_adf") > 0;
  data->app_seed = uniform_hash("vw", 2, 0);
  data->a_s = v_init<action_score>();
  data->all = arg.all;

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
    if (use_cs)
      l = &init_cost_sensitive_learner(data, base, predict_or_learn_adf<true, true>, predict_or_learn_adf<false, true>, arg.all->p, 1);
    else
      l = &init_multiclass_learner(data, base, predict_or_learn_adf<true, false>, predict_or_learn_adf<false, false>, arg.all->p, 1);
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

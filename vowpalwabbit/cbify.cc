#include "reductions.h"
#include "cb_algs.h"
#include "vw.h"
#include "hash.h"
#include "explore.h"

#include <vector>

using namespace LEARNER;
using namespace exploration;
using namespace ACTION_SCORE;
// using namespace COST_SENSITIVE;

using namespace VW::config;

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
  bool use_adf;  // if true, reduce to cb_explore_adf instead of cb_explore
  cbify_adf_data adf_data;
  float loss0;
  float loss1;

  // for ldf inputs
  std::vector<v_array<COST_SENSITIVE::wclass>> cs_costs;
  std::vector<v_array<CB::cb_class>> cb_costs;
  std::vector<ACTION_SCORE::action_scores> cb_as;

  ~cbify()
  {
    CB::cb_label.delete_label(&cb_label);
    a_s.delete_v();

    if (use_adf)
    {
      for (size_t a = 0; a < adf_data.num_actions; ++a)
      {
        adf_data.ecs[a]->pred.a_s.delete_v();
        VW::dealloc_example(CB::cb_label.delete_label, *adf_data.ecs[a]);
        free_it(adf_data.ecs[a]);
      }
      for (auto& as : cb_as) as.delete_v();
    }
  }
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
  {
    if (wc.class_index == final_prediction)
    {
      cost = wc.x;
      break;
    }
  }
  return data.loss0 + (data.loss1 - data.loss0) * cost;
}

float loss_csldf(cbify& data, std::vector<v_array<COST_SENSITIVE::wclass>>& cs_costs, uint32_t final_prediction)
{
  float cost = 0.;
  for (auto costs : cs_costs)
  {
    if (costs[0].class_index == final_prediction)
    {
      cost = costs[0].x;
      break;
    }
  }
  return data.loss0 + (data.loss1 - data.loss0) * cost;
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

    // offset indices for given action
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
  // Store the multiclass or cost-sensitive input label
  MULTICLASS::label_t ld;
  COST_SENSITIVE::label csl;
  if (use_cs)
    csl = ec.l.cs;
  else
    ld = ec.l.multi;

  data.cb_label.costs.clear();
  ec.l.cb = data.cb_label;
  ec.pred.a_s = data.a_s;

  // Call the cb_explore algorithm. It returns a vector of probabilities for each action
  base.predict(ec);
  // data.probs = ec.pred.scalars;

  uint32_t chosen_action;
  if (sample_after_normalizing(
          data.app_seed + data.example_counter++, begin_scores(ec.pred.a_s), end_scores(ec.pred.a_s), chosen_action))
    THROW("Failed to sample from pdf");

  CB::cb_class cl;
  cl.action = chosen_action + 1;
  cl.probability = ec.pred.a_s[chosen_action].score;

  if (!cl.action)
    THROW("No action with non-zero probability found!");
  if (use_cs)
    cl.cost = loss_cs(data, csl.costs, cl.action);
  else
    cl.cost = loss(data, ld.label, cl.action);

  // Create a new cb label
  data.cb_label.costs.push_back(cl);
  ec.l.cb = data.cb_label;

  if (is_learn)
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
  // Store the multiclass or cost-sensitive input label
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
  if (sample_after_normalizing(data.app_seed + data.example_counter++, begin_scores(out_ec.pred.a_s),
          end_scores(out_ec.pred.a_s), chosen_action))
    THROW("Failed to sample from pdf");

  CB::cb_class cl;
  cl.action = out_ec.pred.a_s[chosen_action].action + 1;
  cl.probability = out_ec.pred.a_s[chosen_action].score;

  if (!cl.action)
    THROW("No action with non-zero probability found!");

  if (use_cs)
    cl.cost = loss_cs(data, csl.costs, cl.action);
  else
    cl.cost = loss(data, ld.label, cl.action);

  // add cb label to chosen action
  auto& lab = data.adf_data.ecs[cl.action - 1]->l.cb;
  lab.costs.clear();
  lab.costs.push_back(cl);

  if (is_learn)
    base.learn(data.adf_data.ecs);

  ec.pred.multiclass = cl.action;
}

void init_adf_data(cbify& data, const size_t num_actions)
{
  auto& adf_data = data.adf_data;
  adf_data.num_actions = num_actions;

  adf_data.ecs.resize(num_actions);
  for (size_t a = 0; a < num_actions; ++a)
  {
    adf_data.ecs[a] = VW::alloc_examples(CB::cb_label.label_size, 1);
    auto& lab = adf_data.ecs[a]->l.cb;
    CB::cb_label.default_label(&lab);
    adf_data.ecs[a]->interactions = &data.all->interactions;
  }
}

template <bool is_learn>
void do_actual_learning_ldf(cbify& data, multi_learner& base, multi_ex& ec_seq)
{
  // change label and pred data for cb
  if (data.cs_costs.size() < ec_seq.size())
    data.cs_costs.resize(ec_seq.size());
  if (data.cb_costs.size() < ec_seq.size())
    data.cb_costs.resize(ec_seq.size());
  if (data.cb_as.size() < ec_seq.size())
    data.cb_as.resize(ec_seq.size());
  for (size_t i = 0; i < ec_seq.size(); ++i)
  {
    auto& ec = *ec_seq[i];
    data.cs_costs[i] = ec.l.cs.costs;
    data.cb_costs[i].clear();
    data.cb_as[i].clear();
    ec.l.cb.costs = data.cb_costs[i];
    ec.pred.a_s = data.cb_as[i];
  }

  base.predict(ec_seq);

  auto& out_ec = *ec_seq[0];

  uint32_t chosen_action;
  if (sample_after_normalizing(data.app_seed + data.example_counter++, begin_scores(out_ec.pred.a_s),
          end_scores(out_ec.pred.a_s), chosen_action))
    THROW("Failed to sample from pdf");

  CB::cb_class cl;
  cl.action = out_ec.pred.a_s[chosen_action].action + 1;
  cl.probability = out_ec.pred.a_s[chosen_action].score;

  if (!cl.action)
    THROW("No action with non-zero probability found!");

  cl.cost = loss_csldf(data, data.cs_costs, cl.action);

  // add cb label to chosen action
  data.cb_label.costs.clear();
  data.cb_label.costs.push_back(cl);
  data.cb_costs[cl.action - 1] = ec_seq[cl.action - 1]->l.cb.costs;
  ec_seq[cl.action - 1]->l.cb = data.cb_label;

  base.learn(ec_seq);

  // set cs prediction and reset cs costs
  for (size_t i = 0; i < ec_seq.size(); ++i)
  {
    auto& ec = *ec_seq[i];
    data.cb_as[i] = ec.pred.a_s;  // store action_score vector for later reuse.
    if (i == cl.action - 1)
      data.cb_label = ec.l.cb;
    else
      data.cb_costs[i] = ec.l.cb.costs;
    ec.l.cs.costs = data.cs_costs[i];
    if (i == cl.action - 1)
      ec.pred.multiclass = cl.action;
    else
      ec.pred.multiclass = 0;
  }
}

void output_example(vw& all, example& ec, bool& hit_loss, multi_ex* ec_seq)
{
  COST_SENSITIVE::label& ld = ec.l.cs;
  v_array<COST_SENSITIVE::wclass> costs = ld.costs;

  if (example_is_newline(ec))
    return;
  if (COST_SENSITIVE::ec_is_example_header(ec))
    return;

  all.sd->total_features += ec.num_features;

  float loss = 0.;

  uint32_t predicted_class = ec.pred.multiclass;

  if (!COST_SENSITIVE::cs_label.test_label(&ec.l))
  {
    for (auto const& cost : costs)
    {
      if (hit_loss)
        break;
      if (predicted_class == cost.class_index)
      {
        loss = cost.x;
        hit_loss = true;
      }
    }

    all.sd->sum_loss += loss;
    all.sd->sum_loss_since_last_dump += loss;
  }

  for (int sink : all.final_prediction_sink) all.print(sink, (float)ec.pred.multiclass, 0, ec.tag);

  if (all.raw_prediction > 0)
  {
    std::string outputString;
    std::stringstream outputStringStream(outputString);
    for (size_t i = 0; i < costs.size(); i++)
    {
      if (i > 0)
        outputStringStream << ' ';
      outputStringStream << costs[i].class_index << ':' << costs[i].partial_prediction;
    }
    // outputStringStream << std::endl;
    all.print_text(all.raw_prediction, outputStringStream.str(), ec.tag);
  }

  COST_SENSITIVE::print_update(all, COST_SENSITIVE::cs_label.test_label(&ec.l), ec, ec_seq, false, predicted_class);
}

void output_example_seq(vw& all, multi_ex& ec_seq)
{
  if (ec_seq.empty())
    return;
  all.sd->weighted_labeled_examples += ec_seq[0]->weight;
  all.sd->example_number++;

  bool hit_loss = false;
  for (example* ec : ec_seq) output_example(all, *ec, hit_loss, &(ec_seq));

  if (all.raw_prediction > 0)
  {
    v_array<char> empty = {nullptr, nullptr, nullptr, 0};
    all.print_text(all.raw_prediction, "", empty);
  }
}

void finish_multiline_example(vw& all, cbify&, multi_ex& ec_seq)
{
  if (!ec_seq.empty())
  {
    output_example_seq(all, ec_seq);
    // global_print_newline(all);
  }
  VW::finish_example(all, ec_seq);
}

base_learner* cbify_setup(options_i& options, vw& all)
{
  uint32_t num_actions = 0;
  auto data = scoped_calloc_or_throw<cbify>();
  bool use_cs;

  option_group_definition new_options("Make Multiclass into Contextual Bandit");
  new_options
      .add(make_option("cbify", num_actions)
               .keep()
               .help("Convert multiclass on <k> classes into a contextual bandit problem"))
      .add(make_option("cbify_cs", use_cs).help("consume cost-sensitive classification examples instead of multiclass"))
      .add(make_option("loss0", data->loss0).default_value(0.f).help("loss for correct label"))
      .add(make_option("loss1", data->loss1).default_value(1.f).help("loss for incorrect label"));
  options.add_and_parse(new_options);

  if (!options.was_supplied("cbify"))
    return nullptr;

  data->use_adf = options.was_supplied("cb_explore_adf");
  data->app_seed = uniform_hash("vw", 2, 0);
  data->a_s = v_init<action_score>();
  data->all = &all;

  if (data->use_adf)
    init_adf_data(*data, num_actions);

  if (!options.was_supplied("cb_explore") && !data->use_adf)
  {
    std::stringstream ss;
    ss << num_actions;
    options.insert("cb_explore", ss.str());
  }

  if (data->use_adf)
  {
    options.insert("cb_min_cost", std::to_string(data->loss0));
    options.insert("cb_max_cost", std::to_string(data->loss1));
  }

  if (options.was_supplied("baseline"))
  {
    std::stringstream ss;
    ss << std::max(std::abs(data->loss0), std::abs(data->loss1)) / (data->loss1 - data->loss0);
    options.insert("lr_multiplier", ss.str());
  }

  learner<cbify, example>* l;

  if (data->use_adf)
  {
    multi_learner* base = as_multiline(setup_base(options, all));
    if (use_cs)
      l = &init_cost_sensitive_learner(
          data, base, predict_or_learn_adf<true, true>, predict_or_learn_adf<false, true>, all.p, 1);
    else
      l = &init_multiclass_learner(
          data, base, predict_or_learn_adf<true, false>, predict_or_learn_adf<false, false>, all.p, 1);
  }
  else
  {
    single_learner* base = as_singleline(setup_base(options, all));
    if (use_cs)
      l = &init_cost_sensitive_learner(
          data, base, predict_or_learn<true, true>, predict_or_learn<false, true>, all.p, 1);
    else
      l = &init_multiclass_learner(data, base, predict_or_learn<true, false>, predict_or_learn<false, false>, all.p, 1);
  }
  all.delete_prediction = nullptr;

  return make_base(*l);
}

base_learner* cbifyldf_setup(options_i& options, vw& all)
{
  auto data = scoped_calloc_or_throw<cbify>();
  bool cbify_ldf_option = false;

  option_group_definition new_options("Make csoaa_ldf into Contextual Bandit");
  new_options
      .add(make_option("cbify_ldf", cbify_ldf_option).keep().help("Convert csoaa_ldf into a contextual bandit problem"))
      .add(make_option("loss0", data->loss0).default_value(0.f).help("loss for correct label"))
      .add(make_option("loss1", data->loss1).default_value(1.f).help("loss for incorrect label"));
  options.add_and_parse(new_options);

  if (!options.was_supplied("cbify_ldf"))
    return nullptr;

  data->app_seed = uniform_hash("vw", 2, 0);
  data->all = &all;
  data->use_adf = true;

  if (!options.was_supplied("cb_explore_adf"))
  {
    options.insert("cb_explore_adf", "");
  }
  options.insert("cb_min_cost", std::to_string(data->loss0));
  options.insert("cb_max_cost", std::to_string(data->loss1));

  if (options.was_supplied("baseline"))
  {
    std::stringstream ss;
    ss << std::max(std::abs(data->loss0), std::abs(data->loss1)) / (data->loss1 - data->loss0);
    options.insert("lr_multiplier", ss.str());
  }

  multi_learner* base = as_multiline(setup_base(options, all));
  learner<cbify, multi_ex>& l = init_learner(
      data, base, do_actual_learning_ldf<true>, do_actual_learning_ldf<false>, 1, prediction_type::multiclass);

  l.set_finish_example(finish_multiline_example);
  all.p->lp = COST_SENSITIVE::cs_label;
  all.delete_prediction = nullptr;

  return make_base(l);
}

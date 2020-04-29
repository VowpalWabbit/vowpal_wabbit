#include "reductions.h"
#include "pmf_to_pdf.h"
#include "explore.h"
#include "vw.h"
#include "debug_log.h"

using namespace LEARNER;
using namespace VW;
using namespace VW::config;

VW_DEBUG_ENABLE(false)

namespace VW { namespace pmf_to_pdf
{

  void reduction::transform_prediction(example& ec)
  {
    scores.clear();
    scores.resize(num_actions, 0.f);
    const float continuous_range = max_value - min_value;
    for (uint32_t i = 0; i < num_actions; i++)
    {
      auto& a_s = temp_pred_a_s[i];
      const uint32_t min_h = (std::max)((int)0, static_cast<int>(i) - static_cast<int>(bandwidth));
      const uint32_t max_h = (std::min)(num_actions, i + bandwidth);
      const uint32_t bandwidth_range = max_h - min_h;
      const float continuous_mass = a_s.score * num_actions / ((float)bandwidth_range * continuous_range);
      for (uint32_t j = min_h; j < max_h; j++)
      {
        scores[j] += continuous_mass;
      }
    }
    auto& p_dist = ec.pred.prob_dist;
    p_dist.clear();
    for (uint32_t i = 0; i < num_actions; i++)
    {
      const float action = min_value + i * continuous_range / num_actions;
      p_dist.push_back({action, scores[i]});
    }
    p_dist.push_back({max_value, 0.f});
  }

  reduction::~reduction()
  {
    temp_lbl_cb.costs.delete_v();
    temp_pred_a_s.delete_v();
  }

  void reduction::predict(example& ec)
  {
    swap_restore_cb_label swap_label(ec, temp_lbl_cb);
    {  // scope for saving / restoring prediction
      swap_restore_action_scores_prediction save_prediction(ec, temp_pred_a_s);
      _p_base->predict(ec);
    }
    transform_prediction(ec);
  }

  void reduction::learn(example& ec)
  {
    const float cost = ec.l.cb_cont.costs[0].cost;
    const float prob = ec.l.cb_cont.costs[0].probability;
    const float action_cont = ec.l.cb_cont.costs[0].action;
    const float continuous_range = max_value - min_value;
    const float ac = (action_cont - min_value) * num_actions / continuous_range;
    int ic = (int)floor(ac);
    const bool cond1 = min_value + ic * continuous_range / num_actions <= action_cont;
    const bool cond2 = action_cont < min_value + (ic + 1) * continuous_range / num_actions;

    if (!cond1 || !cond2)
    {
      if (!cond1)
        ic--;

      if (!cond2)
        ic++;
    }

    const uint32_t min_value = (std::max)(0, ic - (int)bandwidth + 1);
    const uint32_t max_value = (std::min)(num_actions - 1, ic + bandwidth);
    swap_restore_cb_label swap_label(ec, temp_lbl_cb);

    ec.l.cb.costs.clear();
    for (uint32_t j = min_value; j <= max_value; j++)
    {
      const uint32_t min_h = (std::max)(0, (int)j - (int)bandwidth);
      const uint32_t max_h = (std::min)(num_actions, j + bandwidth);
      const uint32_t bandwidth_range = max_h - min_h;
      ec.l.cb.costs.push_back({cost, j + 1, prob * bandwidth_range * continuous_range / num_actions, 0.0f});
    }

    swap_restore_action_scores_prediction swap_prediction(ec, temp_pred_a_s);

    _p_base->learn(ec);
  }

  void predict(pmf_to_pdf::reduction& data, single_learner&, example& ec)
  {
    data.predict(ec);
  }

  void learn(pmf_to_pdf::reduction& data, single_learner&, example& ec)
  {
    data.learn(ec);
  }

  void finish(reduction& data)
  {
    data.~reduction();
  }

  void print_update(vw& all, bool is_test, example& ec, std::stringstream& pred_string)
  {
    if (all.sd->weighted_examples() >= all.sd->dump_interval && !all.quiet && !all.bfgs)
    {
      std::stringstream label_string;
      if (is_test)
        label_string << " unknown";
      else
      {
        const auto& cost = ec.l.cb.costs[0];
        label_string << cost.action << ":" << cost.cost << ":" << cost.probability;
      }
      all.sd->print_update(all.holdout_set_off, all.current_pass, label_string.str(), pred_string.str(), ec.num_features,
          all.progress_add, all.progress_arg);
    }
  }

  inline bool observed_cost(CB::cb_class* cl)
  {
    // cost observed for this action if it has non zero probability and cost != FLT_MAX
    return (cl != nullptr && cl->cost != FLT_MAX && cl->probability > .0);
  }

  CB::cb_class* get_observed_cost(CB::label& ld)
  {
    for (auto& cl : ld.costs)
      if (observed_cost(&cl))
        return &cl;
    return nullptr;
  }

  void output_example(vw& all, reduction&, example& ec, CB::label& ld)
  {
    float loss = 0.;

    if (get_observed_cost(ec.l.cb) != nullptr)
      for (auto& cbc : ec.l.cb.costs)
        for (uint32_t i = 0; i < ec.pred.prob_dist.size(); i++)
          loss += (cbc.cost / cbc.probability) * ec.pred.prob_dist[i].value;

    all.sd->update(ec.test_only, get_observed_cost(ld) != nullptr, loss, 1.f, ec.num_features);

    char temp_str[20];
    std::stringstream ss, sso;
    float maxprob = 0.;
    uint32_t maxid = 0;
    for (uint32_t i = 0; i < ec.pred.prob_dist.size(); i++)
    {
      sprintf(temp_str, "%f ", ec.pred.prob_dist[i].value);
      ss << temp_str;
      if (ec.pred.prob_dist[i].value > maxprob)
      {
        maxprob = ec.pred.prob_dist[i].value;
        maxid = i + 1;
      }
    }

    sprintf(temp_str, "%d:%f", maxid, maxprob);
    sso << temp_str;

    for (int sink : all.final_prediction_sink) all.print_text(sink, ss.str(), ec.tag);

    print_update(all, CB::cb_label.test_label(&ld), ec, sso);
  }

  void finish_example(vw& all, reduction& c, example& ec)
  {
    output_example(all, c, ec, ec.l.cb);
    VW::finish_example(all, ec);
  }

  base_learner* pmf_to_pdf_setup(options_i& options, vw& all)
  {
    auto data = scoped_calloc_or_throw<pmf_to_pdf::reduction>();

    option_group_definition new_options("CB Continuous");
    new_options
        .add(make_option("cb_continuous", data->num_actions)
                 .default_value(0)
                 .keep()
                 .help("Convert discrete PDF into continuous PDF."))
        .add(make_option("min_value", data->min_value).keep().help("Minimum continuous value"))
        .add(make_option("max_value", data->max_value).keep().help("Maximum continuous value"))
        .add(make_option("bandwidth", data->bandwidth)
                 .default_value(1)
                 .keep()
                 .help("Bandwidth (radius) of randomization around discrete actions in number of actions."));
    options.add_and_parse(new_options);

    if (data->num_actions == 0)
      return nullptr;
    if (!options.was_supplied("cb_continuous"))
      return nullptr;
    if (!options.was_supplied("cb_explore"))
    {
      std::stringstream ss;
      ss << data->num_actions;
      options.insert("cb_explore", ss.str());
    }
    if (!options.was_supplied("min_value") || !options.was_supplied("max_value"))
    {
      THROW("error: min and max values must be supplied with cb_continuous");
    }
    if (data->bandwidth <= 0)
    {
      THROW("error: Bandwidth must be >= 1");
    }

    auto p_base = as_singleline(setup_base(options, all));
    data->_p_base = p_base;

    learner<reduction, example>& l =
        init_learner(data, p_base, learn, predict, 1, prediction_type::pdf);

    l.set_finish(finish);

    return make_base(l);
  }

}  // namespace pmf_to_pdf
}  // namespace VW

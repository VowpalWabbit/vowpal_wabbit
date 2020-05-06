#include "reductions.h"
#include "pmf_to_pdf.h" // todo rename
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
    const float continuous_range = max_value - min_value;
    const float unit_range = continuous_range / (num_actions - 1);
    const float h = unit_range * bandwidth;

    size_t n = temp_pred_a_s.size();
    assert(n != 0);

    pdf_lim.clear();
    if (temp_pred_a_s[0].action - bandwidth != 0)
      pdf_lim.push_back(0);

    uint32_t l = 0;
    uint32_t r = 0;
    while (l < n || r < n)
    {
      if (l == n || temp_pred_a_s[r].action + bandwidth < temp_pred_a_s[l].action - bandwidth)
        pdf_lim.push_back(temp_pred_a_s[r++].action + bandwidth);
      else if (r == n || temp_pred_a_s[l].action - bandwidth < temp_pred_a_s[r].action + bandwidth)
        pdf_lim.push_back(temp_pred_a_s[l++].action - bandwidth);
      else if (temp_pred_a_s[l].action - bandwidth == temp_pred_a_s[r].action + bandwidth)
      {
        pdf_lim.push_back(temp_pred_a_s[l].action - bandwidth);
        l++; r++;
      }
    }

    if (temp_pred_a_s[n-1].action + bandwidth != num_actions - 1) 
      pdf_lim.push_back(num_actions - 1);

    auto& p_dist = ec.pred.prob_dist;
    p_dist.clear();

    size_t m = pdf_lim.size();
    l = 0; r = 0;
    float p = 0;
    for (uint32_t i = 0; i < m - 1; i++)
    {
      if (l < n && pdf_lim[i] == temp_pred_a_s[l].action - bandwidth)
        p += temp_pred_a_s[l++].score / (2 * h);
      if (r < n && pdf_lim[i] == temp_pred_a_s[r].action + bandwidth)
        p -= temp_pred_a_s[r++].score / (2 * h);
      const float left = min_value + pdf_lim[i] * unit_range;
      const float right = min_value + pdf_lim[i+1] * unit_range;
      p_dist.push_back({left, right, p});
    }
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
    const float unit_range = continuous_range / (num_actions - 1);
    const float h = unit_range * bandwidth;

    const float ac = (action_cont - min_value) / unit_range;
    int ic = (int)floor(ac);
    const bool cond1 = min_value + ic * unit_range <= action_cont;
    const bool cond2 = action_cont < min_value + (ic + 1) * unit_range;

    if (!cond1 || !cond2)
    {
      if (!cond1)
        ic--;
      if (!cond2)
        ic++;
    }

    const uint32_t min_value = (std::max)((int)bandwidth, ic - (int)bandwidth + 1);
    const uint32_t max_value = (std::min)(num_actions - 1 - bandwidth, ic + bandwidth);

    swap_restore_cb_label swap_label(ec, temp_lbl_cb);

    ec.l.cb.costs.clear();
    ec.l.cb.costs.push_back({cost, min_value + 1, prob * 2 * bandwidth * continuous_range / num_actions, 0.0f});
    ec.l.cb.costs.push_back({cost, max_value + 1, prob * 2 * bandwidth * continuous_range / num_actions, 0.0f});

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
          loss += (cbc.cost / cbc.probability) * ec.pred.prob_dist[i].pdf_value;

    all.sd->update(ec.test_only, get_observed_cost(ld) != nullptr, loss, 1.f, ec.num_features);

    char temp_str[20];
    std::stringstream ss, sso;
    float maxprob = 0.;
    uint32_t maxid = 0;
    for (uint32_t i = 0; i < ec.pred.prob_dist.size(); i++)
    {
      sprintf(temp_str, "%f ", ec.pred.prob_dist[i].pdf_value);
      ss << temp_str;
      if (ec.pred.prob_dist[i].pdf_value > maxprob)
      {
        maxprob = ec.pred.prob_dist[i].pdf_value;
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

  base_learner* setup(options_i& options, vw& all)
  {
    auto data = scoped_calloc_or_throw<pmf_to_pdf::reduction>();

    option_group_definition new_options("PMF to PDF");
    new_options
        .add(make_option("pmf_to_pdf", data->num_actions)
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
    if (!options.was_supplied("pmf_to_pdf"))
      return nullptr;
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

    learner<pmf_to_pdf::reduction, example>& l =
        init_learner(data, p_base, learn, predict, 1, prediction_type::pdf);

    l.set_finish(finish);

    return make_base(l);
  }

}  // namespace pmf_to_pdf
}  // namespace VW

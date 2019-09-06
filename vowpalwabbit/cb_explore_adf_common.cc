#include "cb_explore_adf_common.h"
#include "cb_adf.h"

namespace VW
{
namespace cb_explore_adf
{
/*
cb_explore_adf_base::~cb_explore_adf_base()
{
  _gen_cs.pred_scores.costs.delete_v();
}
*/
size_t cb_explore_adf_base::fill_tied(v_array<ACTION_SCORE::action_score>& preds)
{
  if (preds.size() == 0)
    return 0;
  size_t ret = 1;
  for (size_t i = 1; i < preds.size(); ++i)
    if (preds[i].score == preds[0].score)
      ++ret;
    else
      return ret;
  return ret;
}

void cb_explore_adf_base::output_example(vw& all, multi_ex& ec_seq)
{
  if (ec_seq.size() <= 0)
    return;

  size_t num_features = 0;

  float loss = 0.;

  auto& ec = *ec_seq[0];
  ACTION_SCORE::action_scores preds = ec.pred.a_s;

  for (const auto& example : ec_seq)
  {
    num_features += example->num_features;
  }

  bool labeled_example = true;
  if (_known_cost.probability > 0)
  {
    for (uint32_t i = 0; i < preds.size(); i++)
    {
      float l = CB_ALGS::get_cost_estimate(&_known_cost, preds[i].action);
      loss += l * preds[i].score;
    }
  }
  else
    labeled_example = false;

  bool holdout_example = labeled_example;
  for (size_t i = 0; i < ec_seq.size(); i++) holdout_example &= ec_seq[i]->test_only;

  all.sd->update(holdout_example, labeled_example, loss, ec.weight, num_features);

  for (auto sink : all.final_prediction_sink) ACTION_SCORE::print_action_score(sink, ec.pred.a_s, ec.tag);

  if (all.raw_prediction > 0)
  {
    std::string outputString;
    std::stringstream outputStringStream(outputString);
    v_array<CB::cb_class> costs = ec.l.cb.costs;

    for (size_t i = 0; i < costs.size(); i++)
    {
      if (i > 0)
        outputStringStream << ' ';
      outputStringStream << costs[i].action << ':' << costs[i].partial_prediction;
    }
    all.print_text(all.raw_prediction, outputStringStream.str(), ec.tag);
  }

  CB::print_update(all, !labeled_example, ec, &ec_seq, true);
}

void cb_explore_adf_base::output_example_seq(vw& all, multi_ex& ec_seq)
{
  if (ec_seq.size() > 0)
  {
    output_example(all, ec_seq);
    if (all.raw_prediction > 0)
      all.print_text(all.raw_prediction, "", ec_seq[0]->tag);
  }
}

void cb_explore_adf_base::finish_multiline_example(vw& all, multi_ex& ec_seq)
{
  if (ec_seq.size() > 0)
  {
    output_example_seq(all, ec_seq);
    CB_ADF::global_print_newline(all);
  }

  VW::finish_example(all, ec_seq);
}

}  // namespace cb_explore_adf
}  // namespace VW

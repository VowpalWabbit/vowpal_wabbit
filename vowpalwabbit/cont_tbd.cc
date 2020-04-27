#include "cont_tbd.h"
#include "parse_args.h"  // setup_base()
#include "debug_log.h"
#include "err_constants.h"
#include "cb_label_parser.h"

using std::cerr;
using std::endl;
using std::stringstream;

VW_DEBUG_ENABLE(false)

namespace VW { namespace continuous_action {

  namespace lbl_parser {

    //void parse_label(parser* p, shared_data*, void* v, v_array<substring>& words)
    //{
      //auto ld = static_cast<continuous_label*>(v);
      //ld->costs.clear();
      //for (auto word : words)
      //{
      //  continuous_label_elm f{0.f,FLT_MAX,0.f,0.f};
      //  tokenize(':', word, p->parse_name);

      //  if (p->parse_name.empty() || p->parse_name.size() > 3)
      //    THROW("malformed cost specification: " << p->parse_name);

      //  f.action = float_of_substring(p->parse_name[0]);

      //  if (p->parse_name.size() > 1)
      //    f.cost = float_of_substring(p->parse_name[1]);

      //  if (nanpattern(f.cost))
      //    THROW("error NaN cost (" << p->parse_name[1] << " for action: " << p->parse_name[0]);

      //  f.probability = .0;
      //  if (p->parse_name.size() > 2)
      //    f.probability = float_of_substring(p->parse_name[2]);

      //  if (nanpattern(f.probability))
      //    THROW("error NaN probability (" << p->parse_name[2] << " for action: " << p->parse_name[0]);

      //  if (f.probability > 1.0)
      //  {
      //    cerr << "invalid probability > 1 specified for an action, resetting to 1." << endl;
      //    f.probability = 1.0;
      //  }
      //  if (f.probability < 0.0)
      //  {
      //    cerr << "invalid probability < 0 specified for an action, resetting to 0." << endl;
      //    f.probability = .0;
      //  }

      //  ld->costs.push_back(f);
      //}
    //}

    //label_parser cont_tbd_label_parser = {
    //  CB::default_label<continuous_label>,
    //  parse_label,
    //  CB::cache_label<continuous_label, continuous_label_elm>,
    //  CB::read_cached_label<continuous_label,continuous_label_elm>,
    //  CB::delete_label<continuous_label>,
    //  CB::weight, CB::copy_label<continuous_label>,
    //  CB::is_test_label<continuous_label>,
    //  sizeof(continuous_label)};

  }

  //int cont_tbd::predict(example& ec, api_status* status=nullptr)
  //{
  //  VW_DBG(ec) << "cont_tbd::predict(), " << features_to_string(ec) << endl;

  //  // Base reduction will populate a probability distribution.
  //  // It is allocated here
  //  ec.pred.prob_dist = _pred_prob_dist;

  //  // This produces a pdf that we need to sample from
  //  _base->predict(ec);

  //  float chosen_action;
  //  // after having the function that samples the pdf and returns back a continuous action
  //  if (S_EXPLORATION_OK !=
  //      exploration::sample_pdf(_p_rand_state, begin_probs(ec.pred.prob_dist),
  //          end_probs(ec.pred.prob_dist), ec.pred.prob_dist[0].action,
  //          ec.pred.prob_dist[ec.pred.prob_dist.size() - 1].action, chosen_action))
  //  {
  //    RETURN_ERROR(status, sample_pdf_failed);
  //  }

  //  // Save prob_dist in case it was re-allocated in base reduction chain
  //  _pred_prob_dist = ec.pred.prob_dist;

  //  ec.pred.action_pdf.action = chosen_action;
  //  ec.pred.action_pdf.value = get_pdf_value(_pred_prob_dist, chosen_action);

  //  return error_code::success;
  //}

  // Continuous action space predict_or_learn. Non-afd workflow only
  // Receives Regression example as input, sends cb_continuous example to base learn/predict


}}

#include "cont_tbd.h"
#include "parse_args.h"  // setup_base()
#include "debug_log.h"
#include <explore.h>
#include "api_status.h"
#include "err_constants.h"
#include "cb_label_parser.h"
#include "rand48.h"

using std::cerr;
using std::endl;
using std::stringstream;
using LEARNER::single_learner;
using VW::config::option_group_definition;
using VW::config::make_option;
using VW::config::options_i;
using VW::cb_continuous::continuous_label;
using VW::cb_continuous::continuous_label_elm;

namespace VW
{
  void finish_example(vw& all, example& ec);
}

VW_DEBUG_ENABLE(false);

namespace VW { namespace continuous_action {

  struct cont_tbd
  {
    int learn(example& ec, const continuous_label& lbl, api_status* status);
    int predict(example& ec, api_status* status);

    void init(single_learner* p_base, uint64_t* p_rand_state);

    ~cont_tbd();
  private:
    single_learner* _base;
    VW::actions_pdf::pdf _pred_prob_dist;
    uint64_t* _p_rand_state;
  };

  namespace lbl_parser {

    void parse_label(parser* p, shared_data* sd, void* v, v_array<substring>& words)
    {
      auto ld = static_cast<continuous_label*>(v);
      ld->costs.clear();
      for (auto word : words)
      {
        continuous_label_elm f{0.f,FLT_MAX,0.f,0.f};
        tokenize(':', word, p->parse_name);

        if (p->parse_name.empty() || p->parse_name.size() > 3)
          THROW("malformed cost specification: " << p->parse_name);

        f.action = float_of_substring(p->parse_name[0]);

        if (p->parse_name.size() > 1)
          f.cost = float_of_substring(p->parse_name[1]);

        if (nanpattern(f.cost))
          THROW("error NaN cost (" << p->parse_name[1] << " for action: " << p->parse_name[0]);

        f.probability = .0;
        if (p->parse_name.size() > 2)
          f.probability = float_of_substring(p->parse_name[2]);

        if (nanpattern(f.probability))
          THROW("error NaN probability (" << p->parse_name[2] << " for action: " << p->parse_name[0]);

        if (f.probability > 1.0)
        {
          cerr << "invalid probability > 1 specified for an action, resetting to 1." << endl;
          f.probability = 1.0;
        }
        if (f.probability < 0.0)
        {
          cerr << "invalid probability < 0 specified for an action, resetting to 0." << endl;
          f.probability = .0;
        }

        ld->costs.push_back(f);
      }
    }

    label_parser cont_tbd_label_parser = {
      CB::default_label<continuous_label>,
      parse_label,
      CB::cache_label<continuous_label, continuous_label_elm>,
      CB::read_cached_label<continuous_label,continuous_label_elm>,
      CB::delete_label<continuous_label>,
      CB::weight, CB::copy_label<continuous_label>,
      CB::is_test_label<continuous_label>,
      sizeof(continuous_label)};

  }

  int cont_tbd::learn(example& ec, const continuous_label& lbl, api_status* status = nullptr)
  {
    assert(lbl.costs.size() == 1);
    assert(!ec.test_only);

    // only needed for reporting progress
    predict(ec, status);

    // TODO: if (progressive_validation) predict_and_save_prediction;

    // save label now and restore it later
    // const polylabel temp = ec.l;
    // ec.l.cb_cont = lbl;

    VW_DBG(ec) << "cont_tbd::learn(), " << cont_label_to_string(ec) << features_to_string(ec) << endl;
    _base->learn(ec);

    // ec.l = temp;

    return error_code::success;
  }

  int cont_tbd::predict(example& ec, api_status* status=nullptr)
  {
    VW_DBG(ec) << "cont_tbd::predict(), " << features_to_string(ec) << endl;

    // Base reduction will populate a probability distribution.
    // It is allocated here
    ec.pred.prob_dist = _pred_prob_dist;

    // This produces a pdf that we need to sample from
    _base->predict(ec);

    float chosen_action;
    // after having the function that samples the pdf and returns back a continuous action
    if (S_EXPLORATION_OK !=
        exploration::sample_after_normalizing(*_p_rand_state, begin_probs(ec.pred.prob_dist),
            one_to_end_probs(ec.pred.prob_dist), ec.pred.prob_dist[0].action,
            ec.pred.prob_dist[ec.pred.prob_dist.size() - 1].action, chosen_action))
    {
      RETURN_ERROR(status, sample_pdf_failed);
    }

    // Advance the random state
    merand48(*_p_rand_state);

    // Save prob_dist in case it was re-allocated in base reduction chain
    _pred_prob_dist = ec.pred.prob_dist;

    ec.pred.action_pdf.action = chosen_action;
    ec.pred.action_pdf.value = get_pdf_value(_pred_prob_dist, chosen_action);

    return error_code::success;
  }

  void cont_tbd::init(single_learner* p_base, uint64_t* p_rand_state)
  {
    _base = p_base;
    _p_rand_state = p_rand_state;
  }

  cont_tbd::~cont_tbd()
  {
    _pred_prob_dist.delete_v();
  }

  // Continuous action space predict_or_learn. Non-afd workflow only
  // Receives Regression example as input, sends cb_continuous example to base learn/predict
  template <bool is_learn>
  void predict_or_learn(cont_tbd& reduction, single_learner& base, example& ec)
  {
    api_status status;
    if (is_learn)
      reduction.learn(ec,ec.l.cb_cont,&status);
    else
      reduction.predict(ec,&status);

    if (status.get_error_code() != error_code::success)
    {
      cerr << status.get_error_msg() << endl;
    }
  }

  inline bool does_example_have_label(example& ec)
  {
    return (!ec.l.cb_cont.costs.empty()
            && ec.l.cb_cont.costs[0].action != FLT_MAX);
  }

  void print_update_cb_cont(vw& all, example& ec)
  {
    if (all.sd->weighted_examples() >= all.sd->dump_interval && !all.quiet && !all.bfgs)
    {
        all.sd->print_update(
          all.holdout_set_off,
          all.current_pass,
          to_string(ec.l.cb_cont.costs[0]),       // Label
          to_string(ec.pred.action_pdf),  // Prediction
          ec.num_features,
          all.progress_add, all.progress_arg);
    }
  }

  // "average loss" "since last" "example counter" "example weight"
  // "current label" "current predict" "current features"
  void output_example(vw& all, cont_tbd& data, example& ec)
  {
    const auto& cb_cont_costs = ec.l.cb_cont.costs;


    all.sd->update(
      ec.test_only,
      does_example_have_label(ec),
      cb_cont_costs.empty()?0.f:cb_cont_costs[0].cost,
      ec.weight,
      ec.num_features);

    all.sd->weighted_labels += ec.weight;

    print_update_cb_cont(all, ec);
  }

  void output_predictions(v_array<int>& predict_file_descriptors, actions_pdf::pdf_segment& prediction)
  {
    stringstream strm;

    strm << prediction.action << ":"
         << ":" << prediction.value << std::endl;

    const std::string str = strm.str();
    for (const int f : predict_file_descriptors)
    {
      if (f > 0)
      {
        io_buf::write_file_or_socket(f, str.c_str(), str.size());
      }
    }
  }

  void finish_example(vw& all, cont_tbd& data, example& ec)
  {
    // add output example
    output_example(all, data, ec);
    output_predictions(all.final_prediction_sink, ec.pred.action_pdf);
    VW::finish_example(all, ec);
  }


  LEARNER::base_learner* cont_tbd_setup(options_i& options, vw& all)
  {
    option_group_definition new_options("CB continuous actions options");
    int num_actions;
    int cb_continuous_num_actions;
    new_options
        .add(config::make_option("cont_tbd", num_actions).keep().help("Contextual Bandits with continuous actions"))
        .add(make_option("cb_continuous", cb_continuous_num_actions)
                 .default_value(0)
                 .keep()
                 .help("Convert PMF (discrete) into PDF (continuous)."));

    options.add_and_parse(new_options);

    if (!options.was_supplied("cont_tbd"))
      return nullptr;

    if (num_actions <= 0)
      THROW(error_code::num_actions_gt_zero_s);

    if (options.was_supplied("cb_continuous"))
    {
      if (cb_continuous_num_actions != num_actions)
        THROW(error_code::action_counts_disagree_s);
    }
    else
    {
      std::stringstream ss;
      ss << num_actions;
      options.insert("cb_continuous", ss.str());
    }

    LEARNER::base_learner* p_base = setup_base(options, all);
    auto p_reduction = scoped_calloc_or_throw<cont_tbd>();
    p_reduction->init(as_singleline(p_base), &all.random_state);

    LEARNER::learner<cont_tbd, example>& l = init_learner(
      p_reduction,
      as_singleline(p_base),
      predict_or_learn<true>,
      predict_or_learn<false>,
      1,
      prediction_type::prob_dist);

    l.set_finish_example(finish_example);

    all.p->lp = lbl_parser::cont_tbd_label_parser;

    return make_base(l);
  }
}}

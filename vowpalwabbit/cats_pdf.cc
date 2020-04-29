#include "cats_pdf.h"
#include "parse_args.h"
#include "err_constants.h"
#include "api_status.h"
#include "cb_label_parser.h"
#include "debug_log.h"

// Aliases
using LEARNER::single_learner;
using std::endl;
using VW::cb_continuous::continuous_label;
using VW::cb_continuous::continuous_label_elm;
using VW::config::make_option;
using VW::config::option_group_definition;
using VW::config::options_i;

// Enable/Disable indented debug statements
VW_DEBUG_ENABLE(false)

// Forward declarations
namespace VW
{
  void finish_example(vw& all, example& ec);
}

namespace VW
{
namespace continuous_action
{
namespace cats_pdf
{
  ////////////////////////////////////////////////////
  // BEGIN cats_pdf reduction and reduction methods
  struct cats_pdf
  {
    int learn(example& ec, api_status* status);
    int predict(example& ec, api_status* status);

    // TODO: replace with constructor after merge with master
    void init(single_learner* p_base);

   private:
    single_learner* _base = nullptr;
  };

  // Pass through
  int cats_pdf::predict(example& ec, api_status* status = nullptr)
  {
    VW_DBG(ec) << "cats_pdf::predict(), " << features_to_string(ec) << endl;
    _base->predict(ec);
    return error_code::success;
  }

  // Pass through
  int cats_pdf::learn(example& ec, api_status* status = nullptr)
  {
    assert(!ec.test_only);
    predict(ec, status);
    VW_DBG(ec) << "cats_pdf::learn(), " << cont_label_to_string(ec) << features_to_string(ec) << endl;
    _base->learn(ec);
    return error_code::success;
  }

  void cats_pdf::init(single_learner* p_base) { _base = p_base; }

  // Free function to tie function pointers to reduction class methods
  template <bool is_learn>
  void predict_or_learn(cats_pdf& reduction, single_learner&, example& ec)
  {
    api_status status;
    if (is_learn)
      reduction.learn(ec, &status);
    else
      reduction.predict(ec, &status);

    if (status.get_error_code() != error_code::success)
    {
      VW_DBG(ec) << status.get_error_msg() << endl;
    }
  }
  // END cats_pdf reduction and reduction methods
  ////////////////////////////////////////////////////

  ///////////////////////////////////////////////////
  // BEGIN: functions to output progress
  class reduction_output
  {
   public:
    static void report_progress(vw& all, cats_pdf&, example& ec);
    static void output_predictions(v_array<int>& predict_file_descriptors, actions_pdf::pdf_new& prediction);

   private:
    static inline bool does_example_have_label(example& ec);
    static void print_update_cb_cont(vw& all, example& ec);
  };

  // Free function to tie function pointers to output class methods
  void finish_example(vw& all, cats_pdf& data, example& ec)
  {
    // add output example
    reduction_output::report_progress(all, data, ec);
    reduction_output::output_predictions(all.final_prediction_sink, ec.pred.prob_dist_new);
    VW::finish_example(all, ec);
  }

  void reduction_output::output_predictions(
      v_array<int>& predict_file_descriptors, actions_pdf::pdf_new& prediction)
  {
    // output to the prediction to all files
    const std::string str = to_string(prediction, true);
    for (const int f : predict_file_descriptors)
      if (f > 0)
        io_buf::write_file_or_socket(f, str.c_str(), str.size());
  }

  // "average loss" "since last" "example counter" "example weight"
  // "current label" "current predict" "current features"
  void reduction_output::report_progress(vw& all, cats_pdf&, example& ec)
  {
    const auto& cb_cont_costs = ec.l.cb_cont.costs;
    all.sd->update(ec.test_only, does_example_have_label(ec), cb_cont_costs.empty() ? 0.f : cb_cont_costs[0].cost,
        ec.weight, ec.num_features);
    all.sd->weighted_labels += ec.weight;
    print_update_cb_cont(all, ec);
  }

  inline bool reduction_output::does_example_have_label(example& ec)
  {
    return (!ec.l.cb_cont.costs.empty() && ec.l.cb_cont.costs[0].action != FLT_MAX);
  }

  void reduction_output::print_update_cb_cont(vw& all, example& ec)
  {
    if (all.sd->weighted_examples() >= all.sd->dump_interval && !all.quiet && !all.bfgs)
    {
      all.sd->print_update(all.holdout_set_off, all.current_pass,
          to_string(ec.l.cb_cont.costs[0]),  // Label
          to_string(ec.pred.prob_dist_new),      // Prediction
          ec.num_features, all.progress_add, all.progress_arg);
    }
  }

  // END: functions to output progress
  ////////////////////////////////////////////////////

  ////////////////////////////////////////////////////
  // Begin: parse a,c,p,x file format
  namespace lbl_parser
  {
    void parse_label(parser* p, shared_data*, void* v, v_array<substring>& words)
    {
      auto ld = static_cast<continuous_label*>(v);
      ld->costs.clear();
      for (auto word : words)
      {
        continuous_label_elm f{0.f, FLT_MAX, 0.f, 0.f};
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
          std::cerr << "invalid probability > 1 specified for an action, resetting to 1." << endl;
          f.probability = 1.0;
        }
        if (f.probability < 0.0)
        {
          std::cerr << "invalid probability < 0 specified for an action, resetting to 0." << endl;
          f.probability = .0;
        }

        ld->costs.push_back(f);
      }
    }

    label_parser cont_tbd_label_parser = {
        CB::default_label<continuous_label>,
        parse_label,
        CB::cache_label<continuous_label, continuous_label_elm>,
        CB::read_cached_label<continuous_label, continuous_label_elm>,
        CB::delete_label<continuous_label>,
        CB::weight,
        CB::copy_label<continuous_label>,
        CB::is_test_label<continuous_label>,
      sizeof(continuous_label)};
  }

  // End: parse a,c,p,x file format
  ////////////////////////////////////////////////////
  
  // Setup reduction in stack
  LEARNER::base_learner* setup(config::options_i& options, vw& all)
  {
    option_group_definition new_options("Continuous action tree with smoothing");
    int num_actions = 0, pdf_num_actions = 0;
    new_options.add(make_option("cats_pdf", num_actions).keep().help("Continuous action tree with smoothing (pdf)"));

    options.add_and_parse(new_options);

  // If cats reduction was not invoked, don't add anything
    // to the reduction stack;
    if (!options.was_supplied("cats_pdf"))
      return nullptr;

    if (num_actions <= 0)
      THROW(error_code::num_actions_gt_zero_s);

    // cats stack = [cats_pdf -> cb_explore_pdf -> pmf_to_pdf -> get_pmf -> cats_tree ... rest specified by cats_tree]
    if (!options.was_supplied("cb_explore_pdf"))
      options.insert("cb_explore_pdf", "");
    if (!options.was_supplied("pmf_to_pdf"))
      options.insert("pmf_to_pdf", "");
    if (!options.was_supplied("get_pmf"))
      options.insert("get_pmf", "");
    if (!options.was_supplied("cats_tree"))
      options.insert("cats_tree", "");

    LEARNER::base_learner* p_base = setup_base(options, all);
    auto p_reduction = scoped_calloc_or_throw<cats_pdf>();
    p_reduction->init(as_singleline(p_base));

    LEARNER::learner<cats_pdf, example>& l = init_learner(p_reduction, as_singleline(p_base), predict_or_learn<true>,
        predict_or_learn<false>, 1, prediction_type::action_pdf_value);

    l.set_finish_example(finish_example);
    all.p->lp = lbl_parser::cont_tbd_label_parser;
    all.delete_prediction = nullptr;

    return make_base(l);
  }
}  
} 
}  // namespace VW

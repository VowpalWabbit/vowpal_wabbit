#include "cb_explore_pdf.h"
#include "err_constants.h"
#include "api_status.h"
#include "parse_args.h"

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

namespace VW
{
namespace continuous_action
{
  ////////////////////////////////////////////////////
  // BEGIN sample_pdf reduction and reduction methods
  struct cb_explore_pdf
  {
    int learn(example& ec, api_status* status);
    int predict(example& ec, api_status* status);

    void init(single_learner* p_base);
    ~cb_explore_pdf();

    private:
      actions_pdf::pdf _pred_pdf;
      single_learner* _base = nullptr;
  };

  int cb_explore_pdf::learn(example& ec, api_status* status)
  {
    _base->learn(ec);
    return error_code::success;
  }

  int cb_explore_pdf::predict(example& ec, api_status* status)
  {
    {  // predict & restore prediction
      _pred_pdf.clear();
      swap_restore_pdf_prediction restore(ec, _pred_pdf);
      _base->predict(ec);
    }

    // TODO:  create egreedy exploration pdf from base.predict() pdf stored in pred_pdf
    return error_code::success;
  }

  void cb_explore_pdf::init(single_learner* p_base)
  {
    _base = p_base;
    _pred_pdf = v_init<actions_pdf::pdf_segment>();
  }

  cb_explore_pdf::~cb_explore_pdf()
  {
    _pred_pdf.delete_v();
  }

  // Free function to tie function pointers to reduction class methods
  template <bool is_learn>
  void predict_or_learn(cb_explore_pdf& reduction, single_learner&, example& ec)
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

  // END sample_pdf reduction and reduction methods
  ////////////////////////////////////////////////////

  // Setup reduction in stack
  LEARNER::base_learner* cb_explore_pdf_setup(config::options_i& options, vw& all)
  {
    option_group_definition new_options("Continuous actions");
    bool invoked = false;
    float epsilon;
    new_options
        .add(make_option("cb_explore_pdf", invoked).keep().help("Sample a pdf and pick a continuous valued action"))
        .add(make_option("epsilon", epsilon).keep().default_value(0.05f).help("epsilon-greedy exploration"));

    options.add_and_parse(new_options);

    // If reduction was not invoked, don't add anything
    // to the reduction stack;
    if (!options.was_supplied("cb_explore_pdf"))
      return nullptr;

    LEARNER::base_learner* p_base = setup_base(options, all);
    auto p_reduction = scoped_calloc_or_throw<cb_explore_pdf>();
    p_reduction->init(as_singleline(p_base));

    LEARNER::learner<cb_explore_pdf, example>& l = init_learner(p_reduction, as_singleline(p_base),
        predict_or_learn<true>,
        predict_or_learn<false>, 1, prediction_type::pdf);

    return make_base(l);
  }
  }  // namespace continuous_action
}  // namespace VW

#include "reduction_stack.h"

#include "cached_learner.h"
#include "config/options.h"
#include "config/options_name_extractor.h"
#include "global_data.h"  // to get vw struct
#include "learner.h"
#include "simple_label_parser.h"
#include "vw_fwd.h"

// reductions / setup functions
#include "reductions/active.h"
#include "reductions/active_cover.h"
#include "reductions/audit_regressor.h"
#include "reductions/autolink.h"
#include "reductions/automl.h"
#include "reductions/baseline.h"
#include "reductions/baseline_challenger_cb.h"
#include "reductions/bfgs.h"
#include "reductions/binary.h"
#include "reductions/boosting.h"
#include "reductions/bs.h"
#include "reductions/cats.h"
#include "reductions/cats_pdf.h"
#include "reductions/cats_tree.h"
#include "reductions/cb/cb_adf.h"
#include "reductions/cb/cb_algs.h"
#include "reductions/cb/cb_dro.h"
#include "reductions/cb/cb_explore.h"
#include "reductions/cb/cb_explore_adf_bag.h"
#include "reductions/cb/cb_explore_adf_cover.h"
#include "reductions/cb/cb_explore_adf_first.h"
#include "reductions/cb/cb_explore_adf_greedy.h"
#include "reductions/cb/cb_explore_adf_regcb.h"
#include "reductions/cb/cb_explore_adf_rnd.h"
#include "reductions/cb/cb_explore_adf_softmax.h"
#include "reductions/cb/cb_explore_adf_squarecb.h"
#include "reductions/cb/cb_explore_adf_synthcover.h"
#include "reductions/cb/cb_explore_pdf.h"
#include "reductions/cb/cb_sample.h"
#include "reductions/cb/cb_to_cb_adf.h"
#include "reductions/cb/cbify.h"
#include "reductions/cb/warm_cb.h"
#include "reductions/cbzo.h"
#include "reductions/classweight.h"
#include "reductions/conditional_contextual_bandit.h"
#include "reductions/confidence.h"
#include "reductions/count_label.h"
#include "reductions/cs_active.h"
#include "reductions/csoaa.h"
#include "reductions/csoaa_ldf.h"
#include "reductions/ect.h"
#include "reductions/epsilon_decay.h"
#include "reductions/explore_eval.h"
#include "reductions/expreplay.h"
#include "reductions/freegrad.h"
#include "reductions/ftrl.h"
#include "reductions/gd.h"
#include "reductions/gd_mf.h"
#include "reductions/generate_interactions.h"
#include "reductions/get_pmf.h"
#include "reductions/interact.h"
#include "reductions/interaction_ground.h"
#include "reductions/kernel_svm.h"
#include "reductions/lda_core.h"
#include "reductions/log_multi.h"
#include "reductions/lrq.h"
#include "reductions/lrqfa.h"
#include "reductions/marginal.h"
#include "reductions/memory_tree.h"
#include "reductions/metrics.h"
#include "reductions/mf.h"
#include "reductions/multilabel_oaa.h"
#include "reductions/mwt.h"
#include "reductions/nn.h"
#include "reductions/noop.h"
#include "reductions/oaa.h"
#include "reductions/offset_tree.h"
#include "reductions/oja_newton.h"
#include "reductions/plt.h"
#include "reductions/pmf_to_pdf.h"
#include "reductions/print.h"
#include "reductions/recall_tree.h"
#include "reductions/sample_pdf.h"
#include "reductions/scorer.h"
#include "reductions/search/search.h"
#include "reductions/sender.h"
#include "reductions/shared_feature_merger.h"
#include "reductions/slates.h"
#include "reductions/stagewise_poly.h"
#include "reductions/svrg.h"
#include "reductions/topk.h"

void register_reductions(std::vector<reduction_setup_fn>& reductions,
    std::vector<std::tuple<std::string, reduction_setup_fn>>& reduction_stack)
{
  std::map<reduction_setup_fn, std::string> allowlist = {{VW::reductions::gd_setup, "gd"},
      {VW::reductions::ftrl_setup, "ftrl"}, {VW::reductions::sender_setup, "sender"}, {VW::reductions::nn_setup, "nn"},
      {VW::reductions::oaa_setup, "oaa"}, {VW::reductions::scorer_setup, "scorer"},
      {VW::reductions::csldf_setup, "csoaa_ldf"}, {VW::cb_explore_adf::greedy::setup, "cb_explore_adf_greedy"},
      {VW::cb_explore_adf::regcb::setup, "cb_explore_adf_regcb"},
      {VW::reductions::shared_feature_merger_setup, "shared_feature_merger"},
      {VW::reductions::generate_interactions_setup, "generate_interactions"},
      {VW::reductions::count_label_setup, "count_label"}, {cb_to_cb_adf_setup, "cb_to_cbadf"}};

  auto name_extractor = VW::config::options_name_extractor();
  VW::workspace dummy_all(VW::io::create_null_logger());

  VW::cached_learner null_ptr_learner(dummy_all, name_extractor, nullptr);

  for (auto setup_fn : reductions)
  {
    if (allowlist.count(setup_fn)) { reduction_stack.push_back(std::make_tuple(allowlist[setup_fn], setup_fn)); }
    else
    {
      auto base = setup_fn(null_ptr_learner);

      if (base == nullptr) { reduction_stack.push_back(std::make_tuple(name_extractor.generated_name, setup_fn)); }
      else
        THROW("fatal: under register_reduction() all setup functions must return nullptr");
    }
  }
}

void prepare_reductions(std::vector<std::tuple<std::string, reduction_setup_fn>>& reduction_stack)
{
  std::vector<reduction_setup_fn> reductions;

  // Base algorithms
  reductions.push_back(VW::reductions::gd_setup);
  reductions.push_back(VW::reductions::kernel_svm_setup);
  reductions.push_back(VW::reductions::ftrl_setup);
  reductions.push_back(VW::reductions::freegrad_setup);
  reductions.push_back(VW::reductions::svrg_setup);
  reductions.push_back(VW::reductions::sender_setup);
  reductions.push_back(VW::reductions::gd_mf_setup);
  reductions.push_back(VW::reductions::print_setup);
  reductions.push_back(VW::reductions::noop_setup);
  reductions.push_back(VW::reductions::bfgs_setup);
  reductions.push_back(VW::reductions::oja_newton_setup);

  reductions.push_back(VW::reductions::mf_setup);

  reductions.push_back(VW::reductions::generate_interactions_setup);

  // Score Users
  reductions.push_back(VW::reductions::baseline_setup);
  reductions.push_back(VW::reductions::expreplay_setup<'b', simple_label_parser>);
  reductions.push_back(VW::reductions::active_setup);
  reductions.push_back(VW::reductions::active_cover_setup);
  reductions.push_back(VW::reductions::confidence_setup);
  reductions.push_back(VW::reductions::nn_setup);
  reductions.push_back(VW::reductions::marginal_setup);
  reductions.push_back(VW::reductions::autolink_setup);
  reductions.push_back(VW::reductions::lrq_setup);
  reductions.push_back(VW::reductions::lrqfa_setup);
  reductions.push_back(VW::reductions::stagewise_poly_setup);
  reductions.push_back(VW::reductions::scorer_setup);
  reductions.push_back(VW::reductions::lda_setup);
  reductions.push_back(VW::reductions::cbzo_setup);

  // Reductions
  reductions.push_back(VW::reductions::bs_setup);
  reductions.push_back(VW::reductions::binary_setup);

  reductions.push_back(VW::reductions::expreplay_setup<'m', MULTICLASS::mc_label>);
  reductions.push_back(VW::reductions::topk_setup);
  reductions.push_back(VW::reductions::oaa_setup);
  reductions.push_back(VW::reductions::boosting_setup);
  reductions.push_back(VW::reductions::ect_setup);
  reductions.push_back(VW::reductions::log_multi_setup);
  reductions.push_back(VW::reductions::recall_tree_setup);
  reductions.push_back(VW::reductions::memory_tree_setup);
  reductions.push_back(VW::reductions::classweight_setup);
  reductions.push_back(VW::reductions::multilabel_oaa_setup);
  reductions.push_back(VW::reductions::plt_setup);

  reductions.push_back(VW::reductions::cs_active_setup);
  reductions.push_back(VW::reductions::csoaa_setup);
  reductions.push_back(VW::reductions::interact_setup);
  reductions.push_back(VW::reductions::csldf_setup);
  reductions.push_back(cb_algs_setup);
  reductions.push_back(cb_adf_setup);
  reductions.push_back(VW::reductions::interaction_ground_setup);
  reductions.push_back(VW::reductions::mwt_setup);
  reductions.push_back(VW::reductions::cats_tree_setup);
  reductions.push_back(VW::reductions::baseline_challenger_cb_setup);
  reductions.push_back(VW::automl::automl_setup);
  reductions.push_back(cb_explore_setup);
  reductions.push_back(VW::cb_explore_adf::greedy::setup);
  reductions.push_back(VW::cb_explore_adf::softmax::setup);
  reductions.push_back(VW::cb_explore_adf::rnd::setup);
  reductions.push_back(VW::cb_explore_adf::regcb::setup);
  reductions.push_back(VW::cb_explore_adf::squarecb::setup);
  reductions.push_back(VW::cb_explore_adf::synthcover::setup);
  reductions.push_back(VW::cb_explore_adf::first::setup);
  reductions.push_back(VW::cb_explore_adf::cover::setup);
  reductions.push_back(VW::cb_explore_adf::bag::setup);
  reductions.push_back(cb_dro_setup);
  reductions.push_back(cb_sample_setup);
  reductions.push_back(VW::reductions::explore_eval_setup);
  reductions.push_back(VW::reductions::epsilon_decay_setup);
  reductions.push_back(VW::reductions::shared_feature_merger_setup);
  reductions.push_back(VW::reductions::ccb_explore_adf_setup);
  reductions.push_back(VW::reductions::slates_setup);
  // cbify/warm_cb can generate multi-examples. Merge shared features after them
  reductions.push_back(warm_cb_setup);
  reductions.push_back(VW::reductions::get_pmf_setup);
  reductions.push_back(VW::pmf_to_pdf::setup);
  reductions.push_back(VW::continuous_action::cb_explore_pdf_setup);
  reductions.push_back(VW::reductions::cats_pdf_setup);
  reductions.push_back(VW::continuous_action::sample_pdf_setup);
  reductions.push_back(VW::reductions::cats_setup);
  reductions.push_back(cbify_setup);
  reductions.push_back(cbifyldf_setup);
  reductions.push_back(cb_to_cb_adf_setup);
  reductions.push_back(VW::reductions::offset_tree_setup);
  reductions.push_back(VW::reductions::expreplay_setup<'c', COST_SENSITIVE::cs_label>);
  reductions.push_back(VW::reductions::search_setup);
  reductions.push_back(VW::reductions::audit_regressor_setup);
  reductions.push_back(VW::reductions::metrics_setup);
  reductions.push_back(VW::reductions::count_label_setup);

  register_reductions(reductions, reduction_stack);
}

namespace VW
{
default_reduction_stack_setup::default_reduction_stack_setup(VW::workspace& all, VW::config::options_i& options)
{
  // push all reduction functions into the stack
  prepare_reductions(reduction_stack);
  delayed_state_attach(all, options);
}

default_reduction_stack_setup::default_reduction_stack_setup() { prepare_reductions(reduction_stack); }

// this should be reworked, but its setup related to how setup is tied with all object
// which is not applicable to everything
void default_reduction_stack_setup::delayed_state_attach(VW::workspace& all, VW::config::options_i& options)
{
  all_ptr = &all;
  options_impl = &options;
  // populate setup_fn -> name map to be used to lookup names in setup_base
  all.build_setupfn_name_dict(reduction_stack);
}

// this function consumes all the reduction_stack until it's able to construct a base_learner
// same signature/code as the old setup_base(...) from parse_args.cc
VW::LEARNER::base_learner* default_reduction_stack_setup::setup_base_learner()
{
  if (!reduction_stack.empty())
  {
    auto func_map = reduction_stack.back();
    reduction_setup_fn setup_func = std::get<1>(func_map);
    std::string setup_func_name = std::get<0>(func_map);
    reduction_stack.pop_back();

    // 'hacky' way of keeping track of the option group created by the setup_func about to be created
    options_impl->tint(setup_func_name);
    auto base = setup_func(*this);
    options_impl->reset_tint();

    // returning nullptr means that setup_func (any reduction) was not 'enabled' but
    // only added their respective command args and did not add itself into the
    // chain of learners, therefore we call into setup_base again
    if (base == nullptr) { return this->setup_base_learner(); }
    else
    {
      reduction_stack.clear();
      return base;
    }
  }

  return nullptr;
}

std::string default_reduction_stack_setup::get_setupfn_name(reduction_setup_fn setup)
{
  return all_ptr->get_setupfn_name(setup);
}
}  // namespace VW

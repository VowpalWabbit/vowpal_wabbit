#include "reduction_stack.h"
#include "learner.h"
#include "parse_args.h"

namespace VW {
namespace reduction_stack {
  VW::LEARNER::base_learner* pop_reduction(vw& all) {
    auto *ret = all.l;
    if(ret)
    {
      all.l = ret->get_base_reduction();
    }
    return ret;
  }

  void push_reduction(vw& all, VW::LEARNER::base_learner* new_reduction) {
    auto *next_reduction = all.l;
    new_reduction->apply_from(next_reduction, all.reduction_template_map);
    all.l = new_reduction;
  }

  
  struct noop {};

  void noop_single(noop&, VW::LEARNER::single_learner&, example&) {}
  void noop_multi(noop&, VW::LEARNER::multi_learner&, multi_ex&) {}

  template<bool is_learn>
  void passthrough_single(noop&, VW::LEARNER::single_learner& base, example& ex){
    if(is_learn) { base.learn(ex); }
    else { base.predict(ex); }
  }

  template<bool is_learn>
  void passthrough_multi(noop&, VW::LEARNER::multi_learner& base, multi_ex& ex){
    if(is_learn) { base.learn(ex); }
    else { base.predict(ex); }
  }

  VW::LEARNER::base_learner* noop_single_setup(VW::config::options_i&, vw&)
  {
    return make_base(VW::LEARNER::init_learner(noop_single, 1));
  }

  VW::LEARNER::base_learner* noop_multi_setup(VW::config::options_i&, vw&)
  {
    return make_base(VW::LEARNER::init_learner(noop_multi, 1));
  }

  VW::LEARNER::base_learner* passthru_single_setup(VW::config::options_i& options, vw& all)
  {
    auto s = scoped_calloc_or_throw<noop>();
    auto base = VW::LEARNER::as_singleline(setup_base(options, all));
    return make_base(VW::LEARNER::init_learner(s, base, passthrough_single<true>, passthrough_single<false>)); 
  }
  
  VW::LEARNER::base_learner* passthru_multi_setup(VW::config::options_i& options, vw& all)
  {
    auto s = scoped_calloc_or_throw<noop>();
    auto base = VW::LEARNER::as_multiline(setup_base(options, all));
    return make_base(VW::LEARNER::init_learner(s, base, passthrough_multi<true>, passthrough_multi<false>));
  }
  
}
}

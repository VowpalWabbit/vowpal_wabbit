#include "reduction_stack.h"
#include "learner.h"
#include "parse_args.h"
#include "vw.h"
#include "red_python.h"

namespace VW {
  void* pop_reduction(vw* all) {
    auto *ret = all->l;
    if(ret)
    {
      all->l = ret->get_base_reduction();
    }
    return (void*)ret;
  }

  void push_reduction(vw* all, void* reduction) {
    auto *new_reduction = (VW::LEARNER::base_learner*)reduction;
    auto *next_reduction = all->l;
    new_reduction->apply_from(next_reduction, all->reduction_template_map);
    all->l = new_reduction;
  }

  void delete_reduction(void* reduction){
    if(!reduction)
      return;
    auto* r = static_cast<LEARNER::base_learner*>(reduction);
    r->~learner<char, char>();
    free(r);
  }

  class NoopExternalBinding : public RED_PYTHON::ExternalBinding
  {
    void SetBaseLearner(void* learner) {}
    void ActualLearn(example *) {}
    void ActualPredict(example *) {}
    bool ShouldRegisterFinishExample() {return false;}
    void ActualFinishExample(example *) {}
  };

  void create_and_push_custom_reduction(vw* all, const std::string& name, std::unique_ptr<RED_PYTHON::ExternalBinding> custom)
  {
    LEARNER::base_learner* custom_reduction = nullptr;
    if(all->l)
    {
      // not pushing a base reduction
      all->reduction_stack.push_back(reduction_stack::noop_single_setup);
      custom_reduction = red_python_setup(*all->options, *all, name, custom.get());
      // clean up the noop
      delete_reduction((void*)custom_reduction->get_base_reduction());

      // weird and hacky. Need a copy to put into the template map.
      all->reduction_stack.push_back(reduction_stack::noop_single_setup);
      all->reduction_stack.push_back(reduction_stack::passthru_single_setup);
      auto tmp = red_python_setup(*all->options, *all, name, new NoopExternalBinding());
      all->reduction_template_map[tmp->hash_index()] = tmp;

    }
    else
    {
      custom_reduction = red_python_base_setup(*all->options, *all, name, custom.get());
    }
    custom.release();
    push_reduction(all, custom_reduction);
  }

namespace reduction_stack {
  
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

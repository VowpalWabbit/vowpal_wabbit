#include <fstream>
#include <iostream>
#include <cfloat>

#include "../../global_data.h"
#include "../../example.h"
#include "../../constant.h"
#include "../../cb.h"
#include "../../action_score.h"
#include "../../best_constant.h"
#include "parse_flat_example.h"
#include "generated/example_generated.h"

namespace VW {
namespace parsers {
namespace flatbuffer {

void parse_simple_label(vw* all, example* ae, const SimpleLabel* label)
{
  ae->l.simple.label = label->label();
  ae->l.simple.weight = label->weight();
  count_label(all->sd, label->label());
}

void parse_cb_label(example* ae, const CBLabel* label)
{
  ae->l.cb.weight = label->weight();
  for (auto i=0;i<label->costs()->Length();i++){
    CB::cb_class f;
    f.cost = label->costs()->Get(i)->cost();
    f.action = label->costs()->Get(i)->action();
    f.probability = label->costs()->Get(i)->probability();
    f.partial_prediction = label->costs()->Get(i)->partial_pred();
    ae->l.cb.costs.push_back(f);
  }
}

void parse_ccb_label(example* ae, const CCBLabel* label)
{
  ae->l.conditional_contextual_bandit.weight = label->weight();
  if (label->example_type() == 1) ae->l.conditional_contextual_bandit.type = CCB::example_type::shared;
  else if (label->example_type() == 2) ae->l.conditional_contextual_bandit.type = CCB::example_type::action;
  else if (label->example_type() == 3) {
    ae->l.conditional_contextual_bandit.type = CCB::example_type::slot;

    if (label->explicit_included_actions() != nullptr){
      for (auto i=0;i<label->explicit_included_actions()->Length();i++){
        ae->l.conditional_contextual_bandit.explicit_included_actions.push_back(label->explicit_included_actions()->Get(i));
      }
    }
    else if (label->outcome() != nullptr){
      auto& ccb_outcome = *(new CCB::conditional_contextual_bandit_outcome());
      ccb_outcome.cost = label->outcome()->cost();
      ccb_outcome.probabilities = v_init<ACTION_SCORE::action_score>();

      for (auto const& as : *(label->outcome()->probabilities()))
        ccb_outcome.probabilities.push_back({as->action(), as->score()});

      ae->l.conditional_contextual_bandit.outcome = &ccb_outcome;
    }
  else ae->l.conditional_contextual_bandit.type = CCB::example_type::unset;
    // ae->l.conditional_contextual_bandit.outcome->cost = label->outcome()->cost();
  }
}

void parse_cb_eval_label(example* ae, const CB_EVAL_Label* label)
{
  ae->l.cb_eval.action = label->action();
  ae->l.cb_eval.event.weight = label->event()->weight();
  for (auto i=0;i<label->event()->costs()->Length();i++){
    CB::cb_class f;
    f.cost = label->event()->costs()->Get(i)->cost();
    f.action = label->event()->costs()->Get(i)->action();
    f.probability = label->event()->costs()->Get(i)->probability();
    f.partial_prediction = label->event()->costs()->Get(i)->partial_pred();
    ae->l.cb_eval.event.costs.push_back(f);
  }
}

void parse_cs_label(example* ae, const CS_Label* label)
{
  for (auto i=0;i<label->costs()->Length();i++){
    COST_SENSITIVE::wclass f;
    f.x = label->costs()->Get(i)->x();
    f.partial_prediction = label->costs()->Get(i)->partial_pred();
    f.wap_value = label->costs()->Get(i)->wap_value();
    f.class_index = label->costs()->Get(i)->class_index();
    ae->l.cs.costs.push_back(f); 
  }
}

void parse_mc_label(vw* all, example* ae, const MultiClass* label)
{
  uint32_t word = label->label();
  ae->l.multi.label = all->sd->ldict ? (uint32_t)all->sd->ldict->get(std::to_string(word)) : word;
  ae->l.multi.weight = label->weight();
}

void parse_multi_label(example* ae, const MultiLabel* label)
{
  for (auto i=0;i<label->labels()->Length();i++)
    ae->l.multilabels.label_v.push_back(label->labels()->Get(i));
}

void parse_slates_label(example* ae, const Slates_Label* label)
{
  ae->l.slates.weight = label->weight();
  if (label->example_type() == 1) {
    ae->l.slates.labeled = label->labeled();
    ae->l.slates.cost = label->cost();
  }
  else if (label->example_type() == 2) ae->l.slates.slot_id = label->slot();
  else if (label->example_type() == 3) {
    ae->l.slates.labeled = label->labeled();
    ae->l.slates.probabilities = v_init<ACTION_SCORE::action_score>();

    for (auto const& as : *(label->probabilities()))
      ae->l.slates.probabilities.push_back({as->action(), as->score()});    
  }
  else {
    THROW("Example type not understood")
  }
}

void parse_no_label()
{
  // No Label
}
} // flatbuffer
} // parsers
} // VW
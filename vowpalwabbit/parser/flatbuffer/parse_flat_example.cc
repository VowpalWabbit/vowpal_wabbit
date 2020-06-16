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
void read_flatbuffer(vw* all, char* line, size_t len, v_array<example*>& examples)
{
  // Do something
}

int flatbuffer_to_examples(vw* all, v_array<example*>& examples)
{
  all->max_examples = all->data->examples()->size() - 1;
  parse_examples(all, examples, all->data);

  return 1; // Get rid of this
}

void parse_examples(vw* all, v_array<example*>& examples, const ExampleCollection* ec)
{
  parse_example(all, examples[0], ec->examples()->Get(example_index));
  example_index++;
}

void parse_example(vw* all, example* ae, const Example* eg)
{
  all->p->lp.default_label(&ae->l);
  parse_flat_label(all, ae, eg);

  if (flatbuffers::IsFieldPresent(eg, VW::parsers::flatbuffer::Example::VT_TAG)){
    ae->tag = v_init<char>();
    VW::string_view tag(eg->tag()->c_str());
    push_many(ae->tag, tag.begin(), tag.size());
  }

  for (int i=0; i < eg->namespaces()->size(); i++){
    parse_namespaces(all, ae, eg->namespaces()->Get(i));
  }
}

void parse_namespaces(vw* all, example* ae, const Namespace* ns)
{

  if (all->hash_from_names){
    ae->indices.push_back(ns->name()->c_str()[0]);
    c_hash = all->p->hasher(ns->name()->c_str(), ns->name()->Length(), all->hash_seed);
  }
  else ae->indices.push_back(ns->hash());

  features& fs = ae->feature_space[ns->hash()];

  for (int j=0; j<ns->features()->size(); j++){
    parse_features(all, ae, fs, ns->features()->Get(j));
  }
}

void parse_features(vw* all, example* ae, features& fs, const Feature* feature)
{
  if (all->hash_from_names){
    uint64_t word_hash = all->p->hasher(feature->name()->c_str(), feature->name()->Length(), c_hash);
    fs.push_back(feature->value(), word_hash);
  }
  else fs.push_back(feature->value(), feature->hash());
}

void parse_flat_label(vw* all, example* ae, const Example* eg)
{
  VW::parsers::flatbuffer::Label label_type = eg->label_type();

  if (label_type == Label_SimpleLabel){
    auto simple_label = static_cast<const VW::parsers::flatbuffer::SimpleLabel*>(eg->label());
    ae->l.simple.label = simple_label->label();
    ae->l.simple.weight = simple_label->weight();
    count_label(all->sd, simple_label->label());
  }

  else if (label_type == Label_CBLabel){
    auto label = static_cast<const VW::parsers::flatbuffer::CBLabel*>(eg->label());
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
  else if (label_type == Label_CCBLabel){
    auto label = static_cast<const VW::parsers::flatbuffer::CCBLabel*>(eg->label());

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
  else if (label_type == Label_MultiClass){
    auto label = static_cast<const VW::parsers::flatbuffer::MultiClass*>(eg->label());
    uint32_t word = label->label();
    ae->l.multi.label = all->sd->ldict ? (uint32_t)all->sd->ldict->get(std::to_string(word)) : word;
    ae->l.multi.weight = label->weight();
  }
  else if (label_type == Label_MultiLabel){
    auto label = static_cast<const VW::parsers::flatbuffer::MultiLabel*>(eg->label());
    for (auto i=0;i<label->labels()->Length();i++){
      ae->l.multilabels.label_v.push_back(label->labels()->Get(i));
    }
  }
  else if (label_type == Label_CS_Label){
    auto label = static_cast<const VW::parsers::flatbuffer::CS_Label*>(eg->label());

    for (auto i=0;i<label->costs()->Length();i++){
      COST_SENSITIVE::wclass f;
      f.x = label->costs()->Get(i)->x();
      f.partial_prediction = label->costs()->Get(i)->partial_pred();
      f.wap_value = label->costs()->Get(i)->wap_value();
      f.class_index = label->costs()->Get(i)->class_index();
      ae->l.cs.costs.push_back(f); 
    }
  }  
  else if (label_type == Label_CB_EVAL_Label){
    auto label = static_cast<const VW::parsers::flatbuffer::CB_EVAL_Label*>(eg->label());

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
  else if (label_type == Label_Slates_Label) {
    auto label = static_cast<const VW::parsers::flatbuffer::Slates_Label*>(eg->label());

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
  else {
    // No label
  }
}

} //flatbuffer
} // parser
} // VW

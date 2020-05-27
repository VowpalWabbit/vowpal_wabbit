#include <fstream>
#include <iostream>

#include "parse_flat_example.h"
#include "example_generated.h"
#include "global_data.h"
#include "example.h"
#include "constant.h"
#include "cb.h"
#include "action_score.h"

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

  return 31; // Get rid of this
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

void parse_flat_label(vw*all, example* ae, const Example* eg)
{
  auto label_type = eg->label_type();

  if (label_type == Label_SimpleLabel){
    auto simple_label = static_cast<const VW::parsers::flatbuffer::SimpleLabel*>(eg->label());
    ae->l.simple.label = simple_label->label();
    ae->l.simple.weight = simple_label->weight();
  }

  else if (label_type == Label_CBLabel){
    auto label = static_cast<const VW::parsers::flatbuffer::CBLabel*>(eg->label());
    ae->l.cb.weight = label->weight();
    for (auto i=0;i<label->costs()->Length();i++){

      ae->l.cb.costs[i].cost = label->costs()->Get(i)->cost();
      ae->l.cb.costs[i].action = label->costs()->Get(i)->action();
      ae->l.cb.costs[i].probability = label->costs()->Get(i)->probability();
      ae->l.cb.costs[i].partial_prediction = label->costs()->Get(i)->partial_pred();
    }
    
  }
  else if (label_type == Label_CCBLabel){
    auto label = static_cast<const VW::parsers::flatbuffer::CCBLabel*>(eg->label());

    ae->l.conditional_contextual_bandit.weight = label->weight();
    //
    for (auto i=0;i<label->explicit_included_actions()->Length();i++){
      ae->l.conditional_contextual_bandit.explicit_included_actions.push_back(label->explicit_included_actions()->Get(i));
    }

    ae->l.conditional_contextual_bandit.outcome->cost = label->outcome()->cost();
  }
  else if (label_type == Label_MultiClass){
    auto label = static_cast<const VW::parsers::flatbuffer::MultiClass*>(eg->label());
    ae->l.multi.label = label->label();
    ae->l.multi.weight = label->weight();
    // ae->l.multi = label->weight();
  }
  else if (label_type == Label_MultiLabel){
    auto label = static_cast<const VW::parsers::flatbuffer::MultiLabel*>(eg->label());
    for (auto i=0;i<label->labels()->Length();i++){
      ae->l.multilabels.label_v.push_back(label->labels()->Get(i));
    }
    //
  }
  else if (label_type == Label_CS_Label){
    auto label = static_cast<const VW::parsers::flatbuffer::CS_Label*>(eg->label());

    for (auto i=0;i<label->costs()->Length();i++){
      auto to_push = COST_SENSITIVE::wclass();
      to_push.x = label->costs()->Get(i)->x();
      to_push.partial_prediction = label->costs()->Get(i)->partial_pred();
      to_push.wap_value = label->costs()->Get(i)->wap_value();
      to_push.class_index = label->costs()->Get(i)->class_index();
      ae->l.cs.costs.push_back(to_push); // This wont work.
    }
  }  
  else {
    auto label = static_cast<const VW::parsers::flatbuffer::CB_EVAL_Label*>(eg->label());

    ae->l.cb_eval.action = label->action();

    ae->l.cb_eval.event.weight = label->event()->weight();
    for (auto i=0;i<label->event()->costs()->Length();i++){
      auto to_push = CB::cb_class();
      to_push.cost = label->event()->costs()->Get(i)->cost();
      to_push.action = label->event()->costs()->Get(i)->action();
      to_push.probability = label->event()->costs()->Get(i)->probability();
      to_push.partial_prediction = label->event()->costs()->Get(i)->partial_pred();
      ae->l.cb_eval.event.costs.push_back(to_push);
    }
  }
}

} //flatbuffer
} // parser
} // VW
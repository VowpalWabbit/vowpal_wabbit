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
#include "parse_label.h"
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
  all->max_examples = (all->max_examples != std::numeric_limits<size_t>::max()) ? all->max_examples : all->data->examples()->size() - 1;
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

  if (flatbuffers::IsFieldPresent(eg, Example::VT_TAG)){
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
  Label label_type = eg->label_type();

  switch (label_type)
  {
  case Label_SimpleLabel:{
    const SimpleLabel* simple_label = static_cast<const SimpleLabel*>(eg->label());
    parse_simple_label(all, ae, simple_label);
    break;
  }
  case Label_CBLabel:{
    const CBLabel* cb_label = static_cast<const CBLabel*>(eg->label());
    parse_cb_label(ae, cb_label);
    break;
  }
  case Label_CCBLabel:{
    const CCBLabel* ccb_label = static_cast<const CCBLabel*>(eg->label());
    parse_ccb_label(ae, ccb_label);
    break;
  }
  case Label_CB_EVAL_Label:{
    auto cb_eval_label = static_cast<const CB_EVAL_Label*>(eg->label());
    parse_cb_eval_label(ae, cb_eval_label);
    break;
  }
  case Label_CS_Label:{
    auto cs_label = static_cast<const CS_Label*>(eg->label());
    parse_cs_label(ae, cs_label);
    break;
  }
  case Label_MultiClass:{
    auto mc_label = static_cast<const MultiClass*>(eg->label());
    parse_mc_label(all, ae, mc_label);
    break;
  }
  case Label_MultiLabel:{
    auto multi_label = static_cast<const MultiLabel*>(eg->label());
    parse_multi_label(ae, multi_label);
    break;
  }
  case Label_Slates_Label:{
    auto slates_label = static_cast<const Slates_Label*>(eg->label());
    parse_slates_label(ae, slates_label);
    break;
  }
  case Label_no_label:
    // auto label = static_cast<const no_label*>(eg->label());
    parse_no_label();
    break;
  default:
    THROW("Label type in Flatbuffer not understood");
  }
}

} //flatbuffer
} // parser
} // VW

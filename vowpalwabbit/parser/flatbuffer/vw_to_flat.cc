#include <sys/timeb.h>
#include "parse_args.h"
#include "parse_regressor.h"
#include "accumulate.h"
#include "best_constant.h"
#include "vw_exception.h"
#include <fstream>

#include "vw.h"
#include "options.h"
#include "options_boost_po.h"
#include "example_generated.h"
#include "preamble.h"
#include "vw_to_flat.h"
namespace VW
{

void convert_txt_to_flat(vw& all)
{
  flatbuffers::FlatBufferBuilder builder(1024);
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Example>> examplecollection;
  example* v = all.p->ready_parsed_examples.pop();
  while (v!=nullptr)
  {
    // Create Label for current example
    flatbuffers::Offset<void> label;
    VW::parsers::flatbuffer::Label label_type = VW::parsers::flatbuffer::Label_NONE;
    if (all.lp_flat.compare("simple") == 0){
      label = VW::parsers::flatbuffer::CreateSimpleLabel(builder, v->l.simple.label, v->l.simple.weight).Union();
      label_type = VW::parsers::flatbuffer::Label_SimpleLabel;
    }
    // TODO : Change for loop iteration from using int to pointers.
    else if (all.lp_flat.compare("cb") == 0){
      std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::CB_class>> costs;
      for (auto i=0; i<v->l.cb.costs.size(); i++){
        costs.push_back(VW::parsers::flatbuffer::CreateCB_class(builder, v->l.cb.costs[i].cost, v->l.cb.costs[i].action, v->l.cb.costs[i].probability, v->l.cb.costs[i].partial_prediction));
      }
      label = VW::parsers::flatbuffer::CreateCBLabelDirect(builder, v->l.cb.weight, &costs).Union();
      label_type = VW::parsers::flatbuffer::Label_CBLabel;
    }
    else if (all.lp_flat.compare("ccb") == 0){
      std::vector<uint32_t> explicit_included_actions;
      for (auto i=0; i<v->l.conditional_contextual_bandit.explicit_included_actions.size(); i++) 
        explicit_included_actions.push_back(v->l.conditional_contextual_bandit.explicit_included_actions[i]);

      std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::action_score>> action_scores;
      for (auto i=0; i<v->l.conditional_contextual_bandit.outcome->probabilities.size(); i++){
        auto action = v->l.conditional_contextual_bandit.outcome->probabilities[i].action;
        auto score = v->l.conditional_contextual_bandit.outcome->probabilities[i].score;
        action_scores.push_back(VW::parsers::flatbuffer::Createaction_score(builder, action, score));
      }
      auto cost = v->l.conditional_contextual_bandit.outcome->cost;
      auto outcome = VW::parsers::flatbuffer::CreateCCB_outcomeDirect(builder, cost, &action_scores);

      auto e_type = v->l.conditional_contextual_bandit.type;
      auto type = VW::parsers::flatbuffer::CCB_example_type_slot;
      if (e_type == 0) auto type = VW::parsers::flatbuffer::CCB_example_type_unset;
      else if (e_type == 1) auto type = VW::parsers::flatbuffer::CCB_example_type_shared;
      else if (e_type == 2) auto type = VW::parsers::flatbuffer::CCB_example_type_action;
      else auto type = VW::parsers::flatbuffer::CCB_example_type_slot;

      label = VW::parsers::flatbuffer::CreateCCBLabelDirect(builder, type, outcome, &explicit_included_actions, v->l.conditional_contextual_bandit.weight).Union();
      label_type = VW::parsers::flatbuffer::Label_CCBLabel;
    }
    else if (all.lp_flat.compare("multilabel"))
    {
      std::vector<uint32_t> labels;
      for (auto i=0; i<v->l.multilabels.label_v.size(); i++) labels.push_back(v->l.multilabels.label_v[i]);

      label = VW::parsers::flatbuffer::CreateMultiLabelDirect(builder, &labels).Union();
      label_type = VW::parsers::flatbuffer::Label_MultiLabel;
    }
    else if (all.lp_flat.compare("mutliclass"))
    {
      label = VW::parsers::flatbuffer::CreateMultiClass(builder, v->l.multi.label, v->l.multi.weight).Union();
      label_type = VW::parsers::flatbuffer::Label_MultiClass;
    }
    else if (all.lp_flat.compare("cs"))
    {
      std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::wclass>> costs;
      for (auto const& wc : v->l.cs.costs)
      {
        costs.push_back(VW::parsers::flatbuffer::Createwclass(builder, wc.x, wc.partial_prediction, wc.wap_value, wc.partial_prediction));
      }
      label = VW::parsers::flatbuffer::CreateCS_LabelDirect(builder, &costs).Union();
      label_type = VW::parsers::flatbuffer::Label_CS_Label;
    }
    // TODO : Keep else condition for error.
    else {
      std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::CB_class>> costs;
      for (auto i=0; i<v->l.cb_eval.event.costs.size(); i++){
        costs.push_back(VW::parsers::flatbuffer::CreateCB_class(builder, v->l.cb_eval.event.costs[i].cost, v->l.cb_eval.event.costs[i].action, v->l.cb_eval.event.costs[i].probability, v->l.cb_eval.event.costs[i].partial_prediction));
      }

      auto sub_label = CreateCBLabelDirect(builder, v->l.cb_eval.event.weight, &costs);
      label = VW::parsers::flatbuffer::CreateCB_EVAL_Label(builder, v->l.cb_eval.action, sub_label).Union();
      label_type = VW::parsers::flatbuffer::Label_CB_EVAL_Label;
    }
    //Iterate through namespaces to first create features
    uint64_t multiplier = (uint64_t)all.wpp << all.weights.stride_shift();
    if (multiplier != 1)  
      for (features& fs : *v)
        for (auto& j : fs.indicies) j /= multiplier;
    std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Namespace>> namespaces;
    for (namespace_index& ns : v->indices)
    {
      // Skip over constant namespace as that will be assigned while reading flatbuffer again
      if (ns==128) continue;

      std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Feature>> fts;

      for (features::iterator& f : v->feature_space[ns]){
        fts.push_back(VW::parsers::flatbuffer::CreateFeatureDirect(builder, nullptr, f.value(), f.index()));
      }
      namespaces.push_back(VW::parsers::flatbuffer::CreateNamespaceDirect(builder, nullptr, ns, &fts));
    }
    examplecollection.push_back(VW::parsers::flatbuffer::CreateExampleDirect(builder, &namespaces, label_type, label.Union()));

    v = all.p->ready_parsed_examples.pop();
  }

  flatbuffers::Offset<VW::parsers::flatbuffer::ExampleCollection> egcollection = VW::parsers::flatbuffer::CreateExampleCollectionDirect(builder, &examplecollection);

  builder.Finish(egcollection);

  uint8_t *buf = builder.GetBufferPointer();
  int size = builder.GetSize();

  std::ofstream outfile;
  outfile.open(all.flatout, std::ios::binary | std::ios::out);

  // std::cout << *(buf-4) << pre.msg_size <<  "\n";
  outfile.write(reinterpret_cast<char*>(buf), size);
}

namespace parsers {
namespace flatbuffer {

// Specify label type through command line while saving
flatbuffers::Offset<VW::parsers::flatbuffer::Label>
parse_flat_label(vw& all, flatbuffers::FlatBufferBuilder builder, example* v){
  // How do I assign the label?
}
}
}
}
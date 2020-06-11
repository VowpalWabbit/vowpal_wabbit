#include <sys/timeb.h>
#include <fstream>

#include "vw_to_flat.h"
#include "../../vowpalwabbit/vw.h"
#include "../../vowpalwabbit/options.h"
#include "../../vowpalwabbit/parse_args.h"
#include "../../vowpalwabbit/parse_regressor.h"
#include "../../vowpalwabbit/accumulate.h"
#include "../../vowpalwabbit/best_constant.h"
#include "../../vowpalwabbit/vw_exception.h"
#include "../../vowpalwabbit/options_boost_po.h"
#include "../../vowpalwabbit/parser/flatbuffer/preamble.h"
#include "../../vowpalwabbit/parser/flatbuffer/generated/example_generated.h"

void convert_txt_to_flat(vw& all)
{
  flatbuffers::FlatBufferBuilder builder(1024);
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Example>> examplecollection;
  example* v = all.p->ready_parsed_examples.pop();
  int examples = 0;
  while (v!=nullptr)
  {
    // Create Label for current example
    flatbuffers::Offset<void> label;
    VW::parsers::flatbuffer::Label label_type = VW::parsers::flatbuffer::Label_NONE;
    // if (all.label_type.compare("simple") == 0){
    if (all.label_type == label_type_t::nolabel){
      label = VW::parsers::flatbuffer::Createno_label(builder, (uint8_t)'\000').Union();
    }
    // TODO : Change for loop iteration from using int to pointers.
    else if (all.label_type == label_type_t::cb){
      std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::CB_class>> costs;
      for (auto const& cost : v->l.cb.costs){
        costs.push_back(VW::parsers::flatbuffer::CreateCB_class(builder, cost.cost, cost.action, cost.probability, cost.partial_prediction));
      }
      label = VW::parsers::flatbuffer::CreateCBLabelDirect(builder, v->l.cb.weight, &costs).Union();
      label_type = VW::parsers::flatbuffer::Label_CBLabel;
    }
    else if (all.label_type == label_type_t::ccb){
      auto weight = v->l.conditional_contextual_bandit.weight;
      auto e_type = v->l.conditional_contextual_bandit.type;
      label_type = VW::parsers::flatbuffer::Label_CCBLabel;

      if (e_type == 1) {
        auto type = VW::parsers::flatbuffer::CCB_Slates_example_type_shared;
        label = VW::parsers::flatbuffer::CreateCCBLabelDirect(builder, type, 0, nullptr, weight).Union();
      }
      else if (e_type == 2) {
        auto type = VW::parsers::flatbuffer::CCB_Slates_example_type_action;
        label = VW::parsers::flatbuffer::CreateCCBLabelDirect(builder, type, 0, nullptr, weight).Union();
      }
      else if (e_type == 3){
        auto type = VW::parsers::flatbuffer::CCB_Slates_example_type_slot;
        std::vector<uint32_t> explicit_included_actions;
        std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::action_score>> action_scores;
        if (v->l.conditional_contextual_bandit.outcome != nullptr){
          for (auto i=0; i<v->l.conditional_contextual_bandit.outcome->probabilities.size(); i++){
            auto action = v->l.conditional_contextual_bandit.outcome->probabilities[i].action;
            auto score = v->l.conditional_contextual_bandit.outcome->probabilities[i].score;
            action_scores.push_back(VW::parsers::flatbuffer::Createaction_score(builder, action, score));
          }
          auto cost = v->l.conditional_contextual_bandit.outcome->cost;
          auto outcome = VW::parsers::flatbuffer::CreateCCB_outcomeDirect(builder, cost, &action_scores);
          label = VW::parsers::flatbuffer::CreateCCBLabelDirect(builder, type, outcome, nullptr).Union();
        }
        else if (&(v->l.conditional_contextual_bandit.explicit_included_actions) != nullptr){
          for (auto const& action : v->l.conditional_contextual_bandit.explicit_included_actions) 
            explicit_included_actions.push_back(action);
          label = VW::parsers::flatbuffer::CreateCCBLabelDirect(builder, type, 0, &explicit_included_actions).Union();
        }
        else{
          label = VW::parsers::flatbuffer::CreateCCBLabelDirect(builder, type, 0, nullptr, weight).Union();
        }
      }
      else { 
        auto type = VW::parsers::flatbuffer::CCB_Slates_example_type_unset;
        label = VW::parsers::flatbuffer::CreateCCBLabelDirect(builder, type, 0, nullptr, weight).Union();
      }
    }
    else if (all.label_type == label_type_t::multi)
    {
      std::vector<uint32_t> labels;
      for (auto const l : v->l.multilabels.label_v) labels.push_back(l);

      label = VW::parsers::flatbuffer::CreateMultiLabelDirect(builder, &labels).Union();
      label_type = VW::parsers::flatbuffer::Label_MultiLabel;
    }
    else if (all.label_type == label_type_t::mc)
    {
      label = VW::parsers::flatbuffer::CreateMultiClass(builder, v->l.multi.label, v->l.multi.weight).Union();
      label_type = VW::parsers::flatbuffer::Label_MultiClass;
    }
    // else if (all.label_type.compare("cs") == 0)
    else if (all.label_type == label_type_t::cs)
    {
      std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::wclass>> costs;
      for (auto const& wc : v->l.cs.costs)
      {
        costs.push_back(VW::parsers::flatbuffer::Createwclass(builder, wc.x, wc.partial_prediction, wc.wap_value, wc.partial_prediction));
      }
      label = VW::parsers::flatbuffer::CreateCS_LabelDirect(builder, &costs).Union();
      label_type = VW::parsers::flatbuffer::Label_CS_Label;
    }
    // TODO : Keep else condition for Simple Label and add no label
    else if (all.label_type == label_type_t::cb_eval){
      std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::CB_class>> costs;
      for (auto i=0; i<v->l.cb_eval.event.costs.size(); i++){
        costs.push_back(VW::parsers::flatbuffer::CreateCB_class(builder, v->l.cb_eval.event.costs[i].cost, v->l.cb_eval.event.costs[i].action, v->l.cb_eval.event.costs[i].probability, v->l.cb_eval.event.costs[i].partial_prediction));
      }

      auto sub_label = CreateCBLabelDirect(builder, v->l.cb_eval.event.weight, &costs);
      label = VW::parsers::flatbuffer::CreateCB_EVAL_Label(builder, v->l.cb_eval.action, sub_label).Union();
      label_type = VW::parsers::flatbuffer::Label_CB_EVAL_Label;
    }
    else if (all.label_type == label_type_t::slates) {
      std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::action_score>> action_scores;
      float weight = v->l.slates.weight;
      auto e_type = v->l.slates.type;
      label_type = VW::parsers::flatbuffer::Label_Slates_Label;

      if (e_type == 1) {//shared type
        auto type = VW::parsers::flatbuffer::CCB_Slates_example_type_shared;
        label = VW::parsers::flatbuffer::CreateSlates_LabelDirect(builder, type, weight, v->l.slates.labeled, v->l.slates.cost, 0U, nullptr).Union();
      }
      else if (e_type == 2){// action type
        auto type = VW::parsers::flatbuffer::CCB_Slates_example_type_action;
        label = VW::parsers::flatbuffer::CreateSlates_LabelDirect(builder, type, weight, false, 0.0, v->l.slates.slot_id, nullptr).Union();
      }
      else if (e_type == 3) {//slot type 
        auto type = VW::parsers::flatbuffer::CCB_Slates_example_type_slot;
        for (auto const& as : v->l.slates.probabilities){
          action_scores.push_back(VW::parsers::flatbuffer::Createaction_score(builder, as.action, as.score));
        }
        label = VW::parsers::flatbuffer::CreateSlates_LabelDirect(builder, type, weight, v->l.slates.labeled, 0.0, 0U, &action_scores).Union();
      }
      else label = VW::parsers::flatbuffer::CreateSlates_LabelDirect(builder, VW::parsers::flatbuffer::CCB_Slates_example_type_unset, weight, false, 0.0, 0U, nullptr).Union();
    }
    else {
      label = VW::parsers::flatbuffer::CreateSimpleLabel(builder, v->l.simple.label, v->l.simple.weight).Union();
      label_type = VW::parsers::flatbuffer::Label_SimpleLabel;
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

    examples++;
    v = all.p->ready_parsed_examples.pop();
  }

  // label_type_t labelt = all.label_type;
  // all.trace_message << "\nLabel type " << #labelt << std::endl;
  all.trace_message << "Number of examples " << examples-1 << std::endl;

  flatbuffers::Offset<VW::parsers::flatbuffer::ExampleCollection> egcollection = VW::parsers::flatbuffer::CreateExampleCollectionDirect(builder, &examplecollection);

  builder.Finish(egcollection);

  uint8_t *buf = builder.GetBufferPointer();
  int size = builder.GetSize();

  std::ofstream outfile;
  if (all.flatout.empty()) all.flatout = all.data_filename + ".fb";
  outfile.open(all.flatout, std::ios::binary | std::ios::out);

  // std::cout << *(buf-4) << pre.msg_size <<  "\n";
  outfile.write(reinterpret_cast<char*>(buf), size);
  all.trace_message << "Flatbuffer " << all.flatout << " created";
}

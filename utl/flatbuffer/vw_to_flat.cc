#include <sys/timeb.h>
#include <fstream>
#include <vector>

#include "vw_to_flat.h"
#include "../../vowpalwabbit/vw.h"
#include "../../vowpalwabbit/options.h"
#include "../../vowpalwabbit/parse_args.h"
#include "../../vowpalwabbit/parse_regressor.h"
#include "../../vowpalwabbit/accumulate.h"
#include "../../vowpalwabbit/best_constant.h"
#include "../../vowpalwabbit/vw_exception.h"
#include "../../vowpalwabbit/options_boost_po.h"
#include "../../vowpalwabbit/parser/flatbuffer/generated/example_generated.h"

void create_simple_label(example* v, flatbuilder& build, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type)
{
  label = VW::parsers::flatbuffer::CreateSimpleLabel(build._builder, v->l.simple.label, v->l.simple.weight).Union();
  label_type = VW::parsers::flatbuffer::Label_SimpleLabel;
}

void create_cb_label(example* v, flatbuilder& build, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type)
{
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::CB_class>> costs;
  for (auto const& cost : v->l.cb.costs){
    costs.push_back(VW::parsers::flatbuffer::CreateCB_class(build._builder, cost.cost, cost.action, cost.probability, cost.partial_prediction));}
  label = VW::parsers::flatbuffer::CreateCBLabelDirect(build._builder, v->l.cb.weight, &costs).Union();
  label_type = VW::parsers::flatbuffer::Label_CBLabel;
}

void create_ccb_label(example* v, flatbuilder& build, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type)
{
  auto weight = v->l.conditional_contextual_bandit.weight;
  auto e_type = v->l.conditional_contextual_bandit.type;
  label_type = VW::parsers::flatbuffer::Label_CCBLabel;

  if (e_type == 1) {
    auto type = VW::parsers::flatbuffer::CCB_Slates_example_type_shared;
    label = VW::parsers::flatbuffer::CreateCCBLabelDirect(build._builder, type, 0, nullptr, weight).Union();
  }
  else if (e_type == 2) {
    auto type = VW::parsers::flatbuffer::CCB_Slates_example_type_action;
    label = VW::parsers::flatbuffer::CreateCCBLabelDirect(build._builder, type, 0, nullptr, weight).Union();
  }
  else if (e_type == 3){
    auto type = VW::parsers::flatbuffer::CCB_Slates_example_type_slot;
    std::vector<uint32_t> explicit_included_actions;
    std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::action_score>> action_scores;
    if (v->l.conditional_contextual_bandit.outcome != nullptr){
      for (const auto& probability : v->l.conditional_contextual_bandit.outcome->probabilities){
        auto action = probability.action;
        auto score = probability.score;
        action_scores.push_back(VW::parsers::flatbuffer::Createaction_score(build._builder, action, score));
      }
      auto cost = v->l.conditional_contextual_bandit.outcome->cost;
      auto outcome = VW::parsers::flatbuffer::CreateCCB_outcomeDirect(build._builder, cost, &action_scores);
      label = VW::parsers::flatbuffer::CreateCCBLabelDirect(build._builder, type, outcome, nullptr).Union();
    }
    else if (&(v->l.conditional_contextual_bandit.explicit_included_actions) != nullptr){
      for (auto const& action : v->l.conditional_contextual_bandit.explicit_included_actions){ 
        explicit_included_actions.push_back(action);}
      label = VW::parsers::flatbuffer::CreateCCBLabelDirect(build._builder, type, 0, &explicit_included_actions).Union();
    }
    else{
      label = VW::parsers::flatbuffer::CreateCCBLabelDirect(build._builder, type, 0, nullptr, weight).Union();
    }
  }
  else { 
    auto type = VW::parsers::flatbuffer::CCB_Slates_example_type_unset;
    label = VW::parsers::flatbuffer::CreateCCBLabelDirect(build._builder, type, 0, nullptr, weight).Union();
  }
}

void create_cb_eval_label(example* v, flatbuilder& build, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type)
{
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::CB_class>> costs;
  for (const auto& cost : v->l.cb_eval.event.costs){
    costs.push_back(VW::parsers::flatbuffer::CreateCB_class(build._builder, cost.cost, cost.action, cost.probability, cost.partial_prediction));
  }
  auto sub_label = CreateCBLabelDirect(build._builder, v->l.cb_eval.event.weight, &costs);
  label = VW::parsers::flatbuffer::CreateCB_EVAL_Label(build._builder, v->l.cb_eval.action, sub_label).Union();
  label_type = VW::parsers::flatbuffer::Label_CB_EVAL_Label;
}

void create_mc_label(example* v, flatbuilder& build, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type)
{
  label = VW::parsers::flatbuffer::CreateMultiClass(build._builder, v->l.multi.label, v->l.multi.weight).Union();
  label_type = VW::parsers::flatbuffer::Label_MultiClass;
}

void create_multi_label(example* v, flatbuilder& build, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type)
{
  std::vector<uint32_t> labels;
  for (auto const l : v->l.multilabels.label_v) {labels.push_back(l);}

  label = VW::parsers::flatbuffer::CreateMultiLabelDirect(build._builder, &labels).Union();
  label_type = VW::parsers::flatbuffer::Label_MultiLabel;
}

void create_slates_label(example* v, flatbuilder& build, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type)
{
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::action_score>> action_scores;
  float weight = v->l.slates.weight;
  auto e_type = v->l.slates.type;
  label_type = VW::parsers::flatbuffer::Label_Slates_Label;

  if (e_type == 1) {//shared type
    auto type = VW::parsers::flatbuffer::CCB_Slates_example_type_shared;
    label = VW::parsers::flatbuffer::CreateSlates_LabelDirect(build._builder, type, weight, v->l.slates.labeled, v->l.slates.cost, 0U, nullptr).Union();
  }
  else if (e_type == 2){// action type
    auto type = VW::parsers::flatbuffer::CCB_Slates_example_type_action;
    label = VW::parsers::flatbuffer::CreateSlates_LabelDirect(build._builder, type, weight, false, 0.0, v->l.slates.slot_id, nullptr).Union();
  }
  else if (e_type == 3) {//slot type 
    auto type = VW::parsers::flatbuffer::CCB_Slates_example_type_slot;
    for (auto const& as : v->l.slates.probabilities){
      action_scores.push_back(VW::parsers::flatbuffer::Createaction_score(build._builder, as.action, as.score));
    }
    label = VW::parsers::flatbuffer::CreateSlates_LabelDirect(build._builder, type, weight, v->l.slates.labeled, 0.0, 0U, &action_scores).Union();
  }
  else {label = VW::parsers::flatbuffer::CreateSlates_LabelDirect(build._builder, VW::parsers::flatbuffer::CCB_Slates_example_type_unset, weight, false, 0.0, 0U, nullptr).Union();}
}

void create_cs_label(example* v, flatbuilder& build, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type)
{
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::wclass>> costs;
  for (auto const& wc : v->l.cs.costs){
    costs.push_back(VW::parsers::flatbuffer::Createwclass(build._builder, wc.x, wc.partial_prediction, wc.wap_value, wc.class_index));}
  label = VW::parsers::flatbuffer::CreateCS_LabelDirect(build._builder, &costs).Union();
  label_type = VW::parsers::flatbuffer::Label_CS_Label;
}

void create_no_label(example* v, flatbuilder& build, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type)
{
  label = VW::parsers::flatbuffer::Createno_label(build._builder, (uint8_t)'\000').Union();
}

void convert_txt_to_flat(vw& all)
{
  flatbuilder build;
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Example>> examplecollection;
  example* v = all.p->ready_parsed_examples.pop();
  int examples = 0;
  while (v!=nullptr)
  {
    // Create Label for current example
    flatbuffers::Offset<void> label;
    VW::parsers::flatbuffer::Label label_type = VW::parsers::flatbuffer::Label_NONE;
    switch (all.label_type)
    {
    case label_type_t::nolabel:
      create_no_label(v, build, label, label_type);
      break;
    case label_type_t::cb:
      create_cb_label(v, build, label, label_type);
      break;
    case label_type_t::ccb:
      create_ccb_label(v, build, label, label_type);
      break;
    case label_type_t::multi:
      create_multi_label(v, build, label, label_type);
      break;
    case label_type_t::mc:
      create_mc_label(v, build, label, label_type);
      break;
    case label_type_t::cs:
      create_cs_label(v, build, label, label_type);
      break;
    case label_type_t::cb_eval:
      create_cb_eval_label(v, build, label, label_type);
      break;
    case label_type_t::slates:
      create_slates_label(v, build, label, label_type);
      break;
    case label_type_t::simple:
      create_simple_label(v, build, label, label_type);
      break;
    default:
      THROW("Unknown label type");
      break;
    }

    uint64_t multiplier = (uint64_t)all.wpp << all.weights.stride_shift();
    if (multiplier != 1) {
      for (features& fs : *v) {
        for (auto& j : fs.indicies) {j /= multiplier;}
      }
    }
    std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Namespace>> namespaces;
    for (namespace_index& ns : v->indices)
    {
      // Skip over constant namespace as that will be assigned while reading flatbuffer again
      if (ns==128) {continue;}

      std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Feature>> fts;

      for (features::iterator& f : v->feature_space[ns]){
        fts.push_back(VW::parsers::flatbuffer::CreateFeatureDirect(build._builder, nullptr, f.value(), f.index()));
      }
      namespaces.push_back(VW::parsers::flatbuffer::CreateNamespaceDirect(build._builder, nullptr, ns, &fts));
    }
    std::string tag(v->tag.begin(), v->tag.size());

    auto flat_namespaces = build._builder.CreateVector<flatbuffers::Offset<VW::parsers::flatbuffer::Namespace>>(namespaces);
    auto flat_example = VW::parsers::flatbuffer::CreateExample(build._builder, flat_namespaces, label_type, label.Union(), build._builder.CreateString(tag.data()));
    examplecollection.push_back(flat_example);

    examples++;
    v = all.p->ready_parsed_examples.pop();
  }

  // label_type_t labelt = all.label_type;
  // all.trace_message << "\nLabel type " << #labelt << std::endl;
  all.trace_message << "Converted " << examples - 1 << " examples" << std::endl;

  flatbuffers::Offset<VW::parsers::flatbuffer::ExampleCollection> egcollection = VW::parsers::flatbuffer::CreateExampleCollectionDirect(build._builder, &examplecollection);

  build._builder.Finish(egcollection);

  uint8_t *buf = build._builder.GetBufferPointer();
  int size = build._builder.GetSize();

  std::ofstream outfile;
  if (all.flatout.empty()) {all.flatout = all.data_filename + ".fb";}
  outfile.open(all.flatout, std::ios::binary | std::ios::out);

  // std::cout << *(buf-4) << pre.msg_size <<  "\n";
  outfile.write(reinterpret_cast<char*>(buf), size);
  all.trace_message << "Flatbuffer " << all.flatout << " created";
}

#include "vw.net.labels.h"

#include <sstream>

API float SimpleLabelReadFromExample(example* ex, float& weight, float& initial)
{
  label_data* ld = &ex->l.simple;
  float label = ld->label;

  const auto& red_fts = ex->_reduction_features.template get<simple_label_reduction_features>();
  weight = red_fts.weight;
  initial = red_fts.initial;

  return label;
}

API void SimpleLabelUpdateExample(vw_net_native::workspace_context* workspace, example* ex, float label, float* maybe_weight, float* maybe_initial)
{
  label_data* ld = &ex->l.simple;
  ld->label = label;

  if (maybe_weight) ex->weight = *maybe_weight;

  if (maybe_initial)
    {
      auto& red_fts = ex->_reduction_features.template get<simple_label_reduction_features>();
      red_fts.initial = *maybe_initial;
    }

    count_label(workspace->vw->sd, ld->label);
}
  
API CB::cb_class* CbLabelReadFromExampleDangerous(example* ex)
{
  CB::label* ld = &ex->l.cb;

  return (ld->costs.size() > 0) ? &ld->costs[0] : nullptr;
}

API void CbLabelUpdateExample(example* ex, const CB::cb_class* f)
{
  CB::label* ld = &ex->l.cb;

  // TODO: Should we be clearing the costs here? 
  // ld->costs.clear();
  if (f) ld->costs.push_back(*f);
}

API vw_net_native::ERROR_CODE StringLabelParseAndUpdateExample(vw_net_native::workspace_context* workspace, example* ex, const char* label, size_t label_len, VW::experimental::api_status* status)
{
  std::string label_str(label, label_len);

  try
  {
    VW::parse_example_label(*workspace->vw, *ex, label_str);
  }
  CATCH_RETURN_STATUS

  return VW::experimental::error_code::success;
}

API float SharedLabelGetCostConstant()
{
  return FLT_MAX;
}

API uint32_t SharedLabelGetActionId()
{
  return (uint32_t)uniform_hash("shared", 6, 0);
}

API char* ComputeDiffDescriptionSimpleLabels(example* ex1, example* ex2)
{
  label_data* ld1 = &ex1->l.simple;
  label_data* ld2 = &ex2->l.simple;
  auto ex1_initial = ex1->_reduction_features.template get<simple_label_reduction_features>().initial;
  auto ex2_initial = ex2->_reduction_features.template get<simple_label_reduction_features>().initial;

  if ((vw_net_native::FloatEqual(ld1->label, ld2->label) &&
        vw_net_native::FloatEqual(ex1_initial, ex2_initial) &&
        vw_net_native::FloatEqual(ex1->weight, ex2->weight)))
  {
    return nullptr;
  }

  std::stringstream sstream;
  sstream << "Label differ. label " << ld1->label << " vs " << ld2->label << ". initial" << ex1_initial << " vs "
          << ex2_initial << ". weight " << ex1->weight << " vs " << ex2->weight;
  return vw_net_native::stringstream_to_cstr(sstream);
}

API char* ComputeDiffDescriptionCbLabels(example* ex1, example* ex2)
{
  CB::label ld1 = ex1->l.cb;
  CB::label ld2 = ex2->l.cb;

  std::stringstream sstream;
  if (ld1.costs.size() != ld2.costs.size())
  {
    sstream << "Cost size differ: " << ld1.costs.size() << " vs " << ld2.costs.size();
    return vw_net_native::stringstream_to_cstr(sstream);
  }
  else
  {
    for (size_t i = 0; i < ld1.costs.size(); i++)
    {
      CB::cb_class c1 = ld1.costs[i];
      CB::cb_class c2 = ld2.costs[i];

      if (c1.action != c2.action)
      {
        sstream << "Action differ: " << c1.action << " vs " << c2.action;
        return vw_net_native::stringstream_to_cstr(sstream);
      }

      // TODO: Should this be done using FloatEqual? C#/CLI does not seem to do it.
      if (c1.cost != c2.cost)
      {
        sstream << "Cost differ: " << c1.cost << " vs " << c2.cost;
        return vw_net_native::stringstream_to_cstr(sstream);
      }

      // The check here differs in precision from FloatEqual, but keep this one to
      // be consistent with the C#/CLI implementation.
      if (abs(c1.probability - c2.probability) / std::max(c1.probability, c2.probability) > 0.01)
      {
        sstream << "Probability differ: " << c1.probability << " vs " << c2.probability;
        return vw_net_native::stringstream_to_cstr(sstream);
      }
    }
  }

  return nullptr;
}


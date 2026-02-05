#pragma once

#include "vw.net.native.h"
#include "vw/core/vw.h"

namespace vw_net_native
{
struct scalar_confidence_t
{
  float value;
  float confidence;
};

struct action_pdf_value_t
{
  float action;
  float pdf_value;
};
}  // namespace vw_net_native

extern "C"
{
  API float GetPredictionScalar(VW::example* ex);
  API vw_net_native::scalar_confidence_t GetPredictionScalarConfidence(VW::workspace* vw, VW::example* ex);
  API size_t GetPredictionScalarsCount(VW::workspace* vw, VW::example* ex);
  API vw_net_native::dotnet_size_t GetPredictionScalars(
      VW::workspace* vw, VW::example* ex, float* values, vw_net_native::dotnet_size_t count);
  API float GetPredictionProb(VW::workspace* vw, VW::example* ex);
  API float GetPredictionCostSensitive(VW::workspace* vw, VW::example* ex);
  API uint32_t GetPredictionMulticlassClass(VW::workspace* vw, VW::example* ex);
  API size_t GetPredictionMultilabelCount(VW::workspace* vw, VW::example* ex);
  API vw_net_native::dotnet_size_t GetPredictionMultilabel(
      VW::workspace* vw, VW::example* ex, uint32_t* values, vw_net_native::dotnet_size_t count);
  API size_t GetPredictionActionScoresCount(VW::workspace* vw, VW::example* ex);
  API vw_net_native::dotnet_size_t GetPredictionActionScores(
      VW::workspace* vw, VW::example* ex, VW::action_score* values, vw_net_native::dotnet_size_t count);
  API size_t GetPredictionTopicProbsCount(VW::workspace* vw, VW::example* ex);
  API vw_net_native::dotnet_size_t GetPredictionTopicProbs(
      VW::workspace* vw, VW::example* ex, float* values, vw_net_native::dotnet_size_t count);
  API uint32_t GetPredictionActiveMulticlassClass(VW::workspace* vw, VW::example* ex);
  API size_t GetPredictionActiveMulticlassMoreInfoRequiredClassesCount(VW::workspace* vw, VW::example* ex);
  API vw_net_native::dotnet_size_t GetPredictionActiveMulticlassMoreInfoRequiredClasses(
      VW::workspace* vw, VW::example* ex, int32_t* values, vw_net_native::dotnet_size_t count);
  API vw_net_native::action_pdf_value_t GetPredictionActionPdfValue(VW::workspace* vw, VW::example* ex);
}

#pragma once

#include "vw.net.native.h"
#include "vw.h"

namespace vw_net_native { 
  struct scalar_confidence_t
  {
    float value;
    float confidence;
  };
}

extern "C" {
  API float GetPredictionScalar(example* ex);
  API vw_net_native::scalar_confidence_t GetPredictionScalarConfidence(VW::workspace* vw, example* ex);
  API size_t GetPredictionScalarsCount(VW::workspace* vw, example* ex);
  API vw_net_native::dotnet_size_t GetPredictionScalars(VW::workspace* vw, example* ex, float* values, vw_net_native::dotnet_size_t count);
  API float GetPredictionProb(VW::workspace* vw, example* ex); 
  API float GetPredictionCostSensitive(VW::workspace* vw, example* ex);
  API uint32_t GetPredictionMulticlassClass(VW::workspace* vw, example* ex);
  API size_t GetPredictionMultilabelCount(VW::workspace* vw, example* ex);
  API vw_net_native::dotnet_size_t GetPredictionMultilabel(VW::workspace* vw, example* ex, uint32_t* values, vw_net_native::dotnet_size_t count);
  API size_t GetPredictionActionScoresCount(VW::workspace* vw, example* ex);
  API vw_net_native::dotnet_size_t GetPredictionActionScores(VW::workspace* vw, example* ex, ACTION_SCORE::action_score* values, vw_net_native::dotnet_size_t count);
  API size_t GetPredictionTopicProbsCount(VW::workspace* vw, example* ex);
  API vw_net_native::dotnet_size_t GetPredictionTopicProbs(VW::workspace* vw, example* ex, float* values, vw_net_native::dotnet_size_t count);
  API uint32_t GetPredictionActiveMulticlassClass(VW::workspace* vw, example* ex);
  API size_t GetPredictionActiveMulticlassMoreInfoRequiredClassesCount(VW::workspace* vw, example* ex);
  API vw_net_native::dotnet_size_t GetPredictionActiveMulticlassMoreInfoRequiredClasses(VW::workspace* vw, example* ex, int32_t* values, vw_net_native::dotnet_size_t count);
}

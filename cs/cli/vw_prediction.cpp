// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw_prediction.h"
#include "vw_example.h"
#include "vw_base.h"
#include "vowpalwabbit.h"
#include "vw/core/learner.h"

namespace VW
{
void CheckExample(VW::workspace* vw, example* ex, prediction_type_t type)
{ if (vw == nullptr)
    throw gcnew ArgumentNullException("vw");

  if (ex == nullptr)
    throw gcnew ArgumentNullException("ex");

  auto ex_pred_type = vw->l->get_output_prediction_type();
  if (ex_pred_type != type)
  { auto sb = gcnew StringBuilder();
    sb->Append("Prediction type must be ");
    // Note: we know this is a static lifetime string constant that is null terminated.
    sb->Append(gcnew String(VW::to_string(type).data()));
    sb->Append(" but is ");
    // Note: we know this is a static lifetime string constant that is null terminated.
    sb->Append(gcnew String(VW::to_string(ex_pred_type).data()));

    throw gcnew ArgumentException(sb->ToString());
  }
}

float VowpalWabbitScalarPredictionFactory::Create(VW::workspace* vw, example* ex)
{ CheckExample(vw, ex, PredictionType);

  try
  { return VW::get_prediction(ex);
  }
  CATCHRETHROW
}


VowpalWabbitScalar VowpalWabbitScalarConfidencePredictionFactory::Create(VW::workspace* vw, example* ex)
{ CheckExample(vw, ex, PredictionType);

  try
  { VowpalWabbitScalar ret;

    ret.Value = VW::get_prediction(ex);
    ret.Confidence = ex->confidence;

    return ret;
  }
  CATCHRETHROW
}

cli::array<float>^ VowpalWabbitScalarsPredictionFactory::Create(VW::workspace* vw, example* ex)
{ CheckExample(vw, ex, PredictionType);

  try
  { auto& scalars = ex->pred.scalars;
    auto values = gcnew cli::array<float>((int)scalars.size());
    int index = 0;
    for (float s : scalars)
      values[index++] = s;

    return values;
  }
  CATCHRETHROW
}

float VowpalWabbitProbabilityPredictionFactory::Create(VW::workspace* vw, example* ex)
{ CheckExample(vw, ex, PredictionType);

  return ex->pred.prob;
}

float VowpalWabbitCostSensitivePredictionFactory::Create(VW::workspace* vw, example* ex)
{ CheckExample(vw, ex, PredictionType);

  try
  { return VW::get_cost_sensitive_prediction(ex);
  }
  CATCHRETHROW
}

Dictionary<int, float>^ VowpalWabbitMulticlassProbabilitiesPredictionFactory::Create(VW::workspace* vw, example* ex)
{
#if _DEBUG
  if (ex == nullptr)
    throw gcnew ArgumentNullException("ex");
#endif
  v_array<float> confidence_scores;

  try
  { confidence_scores = VW::get_cost_sensitive_prediction_confidence_scores(ex);
  }
  CATCHRETHROW

  auto values = gcnew Dictionary<int, float>();
  int i = 0;
  for (auto& val : confidence_scores)
  { values->Add(++i, val);
  }

  return values;
}

uint32_t VowpalWabbitMulticlassPredictionFactory::Create(VW::workspace* vw, example* ex)
{ CheckExample(vw, ex, PredictionType);

  return ex->pred.multiclass;
}

cli::array<int>^ VowpalWabbitMultilabelPredictionFactory::Create(VW::workspace* vw, example* ex)
{ CheckExample(vw, ex, prediction_type_t::MULTILABELS);

  size_t length;
  uint32_t* labels;

  try
  { labels = VW::get_multilabel_predictions(ex, length);
  }
  CATCHRETHROW

  if (length > Int32::MaxValue)
    throw gcnew ArgumentOutOfRangeException("Multi-label predictions too large");

  auto values = gcnew cli::array<int>((int)length);

  if (length > 0)
    Marshal::Copy(IntPtr(labels), values, 0, (int)length);

  return values;
}

cli::array<ActionScore>^ VowpalWabbitActionScoreBasePredictionFactory::Create(VW::workspace* vw, example* ex)
{ CheckExample(vw, ex, PredictionType);

  auto& a_s = ex->pred.a_s;
  auto values = gcnew cli::array<ActionScore>((int)a_s.size());

  auto index = 0;
  for (auto& as : a_s)
  { values[index].Action = as.action;
    values[index].Score = as.score;
    index++;
  }

  return values;
}

cli::array<float>^ VowpalWabbitTopicPredictionFactory::Create(VW::workspace* vw, example* ex)
{ if (ex == nullptr)
    throw gcnew ArgumentNullException("ex");

  auto values = gcnew cli::array<float>(vw->reduction_state.lda);
  Marshal::Copy(IntPtr(ex->pred.scalars.begin()), values, 0, vw->reduction_state.lda);

  return values;
}

VowpalWabbitActiveMulticlass^ VowpalWabbitActiveMulticlassPredictionFactory::Create(VW::workspace* vw, example* ex)
{
  CheckExample(vw, ex, prediction_type_t::ACTIVE_MULTICLASS);
  auto struct_obj = gcnew VowpalWabbitActiveMulticlass();
  const auto length = ex->pred.active_multiclass.more_info_required_for_classes.size();
  struct_obj->more_info_required_for_classes = gcnew cli::array<int>((int)length);

  if (length > 0)
  {
    Marshal::Copy(IntPtr(ex->pred.active_multiclass.more_info_required_for_classes.data()),
        struct_obj->more_info_required_for_classes, 0, (int)length);
  }

  struct_obj->predicted_class = ex->pred.active_multiclass.predicted_class;

  return struct_obj;
}

System::Object^ VowpalWabbitDynamicPredictionFactory::Create(VW::workspace* vw, example* ex)
{ if (ex == nullptr)
    throw gcnew ArgumentNullException("ex");

  switch (vw->l->get_output_prediction_type())
  { case prediction_type_t::SCALAR:
      return VowpalWabbitPredictionType::Scalar->Create(vw, ex);
    case prediction_type_t::SCALARS:
      return VowpalWabbitPredictionType::Scalars->Create(vw, ex);
    case prediction_type_t::MULTICLASS:
      return VowpalWabbitPredictionType::Multiclass->Create(vw, ex);
    case prediction_type_t::MULTILABELS:
      return VowpalWabbitPredictionType::Multilabel->Create(vw, ex);
    case prediction_type_t::ACTION_SCORES:
      return VowpalWabbitPredictionType::ActionScore->Create(vw, ex);
    case prediction_type_t::ACTION_PROBS:
      return VowpalWabbitPredictionType::ActionProbabilities->Create(vw, ex);
    case prediction_type_t::PROB:
      return VowpalWabbitPredictionType::Probability->Create(vw, ex);
    case prediction_type_t::MULTICLASS_PROBS:
      return VowpalWabbitPredictionType::MultiClassProbabilities->Create(vw, ex);
    case prediction_type_t::ACTIVE_MULTICLASS:
      return VowpalWabbitPredictionType::ActiveMulticlass->Create(vw, ex);
    default:
    { auto sb = gcnew StringBuilder();
      sb->Append("Unsupported prediction type: ");
      // Note: we know this is a static lifetime string constant that is null terminated.
      sb->Append(gcnew String(VW::to_string(vw->l->get_output_prediction_type()).data()));
      throw gcnew ArgumentException(sb->ToString());
    }
  }
}
}

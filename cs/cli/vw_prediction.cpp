// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw_prediction.h"
#include "vw_example.h"
#include "vw_base.h"
#include "vowpalwabbit.h"

namespace VW
{
void CheckExample(vw* vw, example* ex, prediction_type_t type)
{ if (vw == nullptr)
    throw gcnew ArgumentNullException("vw");

  if (ex == nullptr)
    throw gcnew ArgumentNullException("ex");

  auto ex_pred_type = vw->l->pred_type;
  if (ex_pred_type != type)
  { auto sb = gcnew StringBuilder();
    sb->Append("Prediction type must be ");
    sb->Append(gcnew String(to_string(type)));
    sb->Append(" but is ");
    sb->Append(gcnew String(to_string(ex_pred_type)));

    throw gcnew ArgumentException(sb->ToString());
  }
}

float VowpalWabbitScalarPredictionFactory::Create(vw* vw, example* ex)
{ CheckExample(vw, ex, PredictionType);

  try
  { return VW::get_prediction(ex);
  }
  CATCHRETHROW
}


VowpalWabbitScalar VowpalWabbitScalarConfidencePredictionFactory::Create(vw* vw, example* ex)
{ CheckExample(vw, ex, PredictionType);

  try
  { VowpalWabbitScalar ret;

    ret.Value = VW::get_prediction(ex);
    ret.Confidence = ex->confidence;

    return ret;
  }
  CATCHRETHROW
}

cli::array<float>^ VowpalWabbitScalarsPredictionFactory::Create(vw* vw, example* ex)
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

float VowpalWabbitProbabilityPredictionFactory::Create(vw* vw, example* ex)
{ CheckExample(vw, ex, PredictionType);

  return ex->pred.prob;
}

float VowpalWabbitCostSensitivePredictionFactory::Create(vw* vw, example* ex)
{ CheckExample(vw, ex, PredictionType);

  try
  { return VW::get_cost_sensitive_prediction(ex);
  }
  CATCHRETHROW
}

Dictionary<int, float>^ VowpalWabbitMulticlassProbabilitiesPredictionFactory::Create(vw* vw, example* ex)
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

uint32_t VowpalWabbitMulticlassPredictionFactory::Create(vw* vw, example* ex)
{ CheckExample(vw, ex, PredictionType);

  return ex->pred.multiclass;
}

cli::array<int>^ VowpalWabbitMultilabelPredictionFactory::Create(vw* vw, example* ex)
{ CheckExample(vw, ex, prediction_type_t::multilabels);

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

cli::array<ActionScore>^ VowpalWabbitActionScoreBasePredictionFactory::Create(vw* vw, example* ex)
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

cli::array<float>^ VowpalWabbitTopicPredictionFactory::Create(vw* vw, example* ex)
{ if (ex == nullptr)
    throw gcnew ArgumentNullException("ex");

  auto values = gcnew cli::array<float>(vw->lda);
  Marshal::Copy(IntPtr(ex->pred.scalars.begin()), values, 0, vw->lda);

  return values;
}

System::Object^ VowpalWabbitDynamicPredictionFactory::Create(vw* vw, example* ex)
{ if (ex == nullptr)
    throw gcnew ArgumentNullException("ex");

  switch (vw->l->pred_type)
  { case prediction_type_t::scalar:
      return VowpalWabbitPredictionType::Scalar->Create(vw, ex);
    case prediction_type_t::scalars:
      return VowpalWabbitPredictionType::Scalars->Create(vw, ex);
    case prediction_type_t::multiclass:
      return VowpalWabbitPredictionType::Multiclass->Create(vw, ex);
    case prediction_type_t::multilabels:
      return VowpalWabbitPredictionType::Multilabel->Create(vw, ex);
    case prediction_type_t::action_scores:
      return VowpalWabbitPredictionType::ActionScore->Create(vw, ex);
    case prediction_type_t::action_probs:
      return VowpalWabbitPredictionType::ActionProbabilities->Create(vw, ex);
    case prediction_type_t::prob:
      return VowpalWabbitPredictionType::Probability->Create(vw, ex);
    case prediction_type_t::multiclassprobs:
      return VowpalWabbitPredictionType::MultiClassProbabilities->Create(vw, ex);
    default:
    { auto sb = gcnew StringBuilder();
      sb->Append("Unsupported prediction type: ");
      sb->Append(gcnew String(to_string(vw->l->pred_type)));
      throw gcnew ArgumentException(sb->ToString());
    }
  }
}
}

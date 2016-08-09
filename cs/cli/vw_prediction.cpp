/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#include "vw_prediction.h"
#include "vw_example.h"
#include "vw_base.h"
#include "vowpalwabbit.h"

namespace VW
{
    float VowpalWabbitScalarPredictionFactory::Create(vw* vw, example* ex)
    {
#if _DEBUG
        if (ex == nullptr)
            throw gcnew ArgumentNullException("ex");
#endif

        try
        {
            return VW::get_prediction(ex);
        }
        CATCHRETHROW
    }

    VowpalWabbitScalar VowpalWabbitScalarConfidencePredictionFactory::Create(vw* vw, example* ex)
    {
#if _DEBUG
      if (ex == nullptr)
        throw gcnew ArgumentNullException("ex");
#endif

      try
      {
        VowpalWabbitScalar ret;

        ret.Value = VW::get_prediction(ex);
        ret.Confidence = ex->confidence;

        return ret;
      }
      CATCHRETHROW
    }

    cli::array<float>^ VowpalWabbitScalarsPredictionFactory::Create(vw* vw, example* ex)
    {
#if _DEBUG
      if (ex == nullptr)
        throw gcnew ArgumentNullException("ex");
#endif

      try
      {
        auto values = gcnew cli::array<float>((int)ex->pred.scalars.size());
        int index = 0;
        for (float s : ex->pred.scalars)
          values[index++] = s;

        return values;
      }
      CATCHRETHROW
    }

    float VowpalWabbitCostSensitivePredictionFactory::Create(vw* vw, example* ex)
    {
#if _DEBUG
        if (ex == nullptr)
            throw gcnew ArgumentNullException("ex");
#endif

        try
        {
            return VW::get_cost_sensitive_prediction(ex);
        }
        CATCHRETHROW
    }

    cli::array<int>^ VowpalWabbitMultilabelPredictionFactory::Create(vw* vw, example* ex)
    {
#if _DEBUG
        if (ex == nullptr)
            throw gcnew ArgumentNullException("ex");
#endif

        size_t length;
        uint32_t* labels;

        try
        {
            labels = VW::get_multilabel_predictions(ex, length);
        }
        CATCHRETHROW

        if (length > Int32::MaxValue)
            throw gcnew ArgumentOutOfRangeException("Multi-label predictions too large");

        auto values = gcnew cli::array<int>((int)length);

        if (length > 0)
            Marshal::Copy(IntPtr(labels), values, 0, (int)length);

        return values;
    }

    cli::array<ActionScore>^ VowpalWabbitActionScorePredictionFactory::Create(vw* vw, example* ex)
    {
#if _DEBUG
      if (ex == nullptr)
        throw gcnew ArgumentNullException("ex");
#endif

      auto values = gcnew cli::array<ActionScore>((int)ex->pred.a_s.size());

      auto index = 0;
      for (auto& as : ex->pred.a_s)
      {
          values[index].Action = as.action;
          values[index].Score = as.score;
          index++;
      }

      return values;
    }

    cli::array<float>^ VowpalWabbitTopicPredictionFactory::Create(vw* vw, example* ex)
    {
#if _DEBUG
        if (vw == nullptr)
            throw gcnew ArgumentNullException("vw");

        if (ex == nullptr)
            throw gcnew ArgumentNullException("ex");
#endif

        auto values = gcnew cli::array<float>(vw->lda);
        Marshal::Copy(IntPtr(ex->topic_predictions.begin()), values, 0, vw->lda);

        return values;
    }
}

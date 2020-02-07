// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw_clr.h"

namespace VW
{
ref class VowpalWabbitExample;
ref class VowpalWabbit;

using namespace System::Collections::Generic;

/// <summary>
/// Interface for prediction factories enabling read-out of various prediction results in an extendable manner.
/// </summary>
generic<typename T>
public interface class IVowpalWabbitPredictionFactory
{
public:
  /// <summary>
  /// Creates a new prediction result from an example and the associated VW instance.
  /// </summary>
  /// <returns>A prediction result.</returns>
  /// <remarks>Implementation must be thread-safe.</remarks>
  T Create(vw* vw, example* ex);

  /// <summary>
  /// Returns the supported prediction type.
  /// </summary>
  property prediction_type_t PredictionType
  { prediction_type_t get();
  }
};

/// <summary>
/// A scalar prediction result.
/// </summary>
public ref class VowpalWabbitDynamicPredictionFactory sealed : IVowpalWabbitPredictionFactory<System::Object^>
{
public:
  /// <summary>
  /// Extracts prediction results from example.
  /// </summary>
  virtual System::Object^ Create(vw* vw, example* ex) sealed;

  /// <summary>
  /// Returns the supported prediction type.
  /// </summary>
  property prediction_type_t PredictionType
  { virtual prediction_type_t get() sealed
    { throw gcnew NotSupportedException("Prediction type is not available.");
    }
  }
};

/// <summary>
/// A scalar prediction result.
/// </summary>
public ref class VowpalWabbitScalarPredictionFactory sealed : IVowpalWabbitPredictionFactory<float>
{
public:
  /// <summary>
  /// Extracts prediction results from example.
  /// </summary>
  virtual float Create(vw* vw, example* ex) sealed;

  /// <summary>
  /// Returns the supported prediction type.
  /// </summary>
  property prediction_type_t PredictionType
  { virtual prediction_type_t get() sealed
    { return prediction_type_t::scalar;
    }
  }
};

public value struct VowpalWabbitScalar
{
public:
  float Value;

  float Confidence;
};

/// <summary>
/// A scalar prediction result.
/// </summary>
public ref class VowpalWabbitScalarConfidencePredictionFactory sealed : IVowpalWabbitPredictionFactory<VowpalWabbitScalar>
{
public:
  /// <summary>
  /// Extracts prediction results from example.
  /// </summary>
  virtual VowpalWabbitScalar Create(vw* vw, example* ex) sealed;

  /// <summary>
  /// Returns the supported prediction type.
  /// </summary>
  property prediction_type_t PredictionType
  { virtual prediction_type_t get() sealed
    { return prediction_type_t::scalar;
    }
  }
};

/// <summary>
/// A scalar prediction result.
/// </summary>
public ref class VowpalWabbitScalarsPredictionFactory sealed : IVowpalWabbitPredictionFactory<cli::array<float>^>
{
public:
  /// <summary>
  /// Extracts prediction results from example.
  /// </summary>
  virtual cli::array<float>^ Create(vw* vw, example* ex) sealed;

  /// <summary>
  /// Returns the supported prediction type.
  /// </summary>
  property prediction_type_t PredictionType
  { virtual prediction_type_t get() sealed
    { return prediction_type_t::scalars;
    }
  }
};

/// <summary>
/// A scalar prediction result.
/// </summary>
public ref class VowpalWabbitProbabilityPredictionFactory sealed : IVowpalWabbitPredictionFactory<float>
{
public:
  /// <summary>
  /// Extracts prediction results from example.
  /// </summary>
  virtual float Create(vw* vw, example* ex) sealed;

  /// <summary>
  /// Returns the supported prediction type.
  /// </summary>
  property prediction_type_t PredictionType
  { virtual prediction_type_t get() sealed
    { return prediction_type_t::prob;
    }
  }
};

/// <summary>
/// A cost sensitive prediction result.
/// </summary>
public ref class VowpalWabbitCostSensitivePredictionFactory sealed : IVowpalWabbitPredictionFactory<float>
{
public:
  /// <summary>
  /// Extracts cost sensitive prediction results from example.
  /// </summary>
  virtual float Create(vw* vw, example* ex) sealed;

  /// <summary>
  /// Returns the supported prediction type.
  /// </summary>
  property prediction_type_t PredictionType
  { virtual prediction_type_t get() sealed
    { return prediction_type_t::multiclass;
    }
  }
};

/// <summary>
/// A cost sensitive prediction result.
/// </summary>
public ref class VowpalWabbitMulticlassPredictionFactory sealed : IVowpalWabbitPredictionFactory<uint32_t>
{
public:
  /// <summary>
  /// Extracts cost sensitive prediction results from example.
  /// </summary>
  virtual uint32_t Create(vw* vw, example* ex) sealed;

  /// <summary>
  /// Returns the supported prediction type.
  /// </summary>
  property prediction_type_t PredictionType
  { virtual prediction_type_t get() sealed
    { return prediction_type_t::multiclass;
    }
  }
};

/// <summary>
/// A cost sensitive prediction result with associated confidence score
/// For -oaa --probabilities
/// </summary>
public ref class VowpalWabbitMulticlassProbabilitiesPredictionFactory sealed : IVowpalWabbitPredictionFactory<Dictionary<int, float>^>
{
public:
  /// <summary>
  /// Extracts cost sensitive prediction results from example, including confidence score.
  /// </summary>
  virtual Dictionary<int, float>^ Create(vw* vw, example* ex) sealed;

  /// <summary>
  /// Returns the supported prediction type.
  /// </summary>
  property prediction_type_t PredictionType
  { virtual prediction_type_t get() sealed
    { return prediction_type_t::multiclassprobs;
    }
  }
};

/// <summary>
/// A multi label prediction result.
/// </summary>
public ref class VowpalWabbitMultilabelPredictionFactory sealed : IVowpalWabbitPredictionFactory<cli::array<int>^>
{
public:
  /// <summary>
  /// Extracts multilabel prediction results from example.
  /// </summary>
  virtual cli::array<int>^ Create(vw* vw, example* ex) sealed;

  /// <summary>
  /// Returns the supported prediction type.
  /// </summary>
  property prediction_type_t PredictionType
  { virtual prediction_type_t get() sealed
    { return prediction_type_t::multilabels;
    }
  }
};

[System::Diagnostics::DebuggerDisplay("{Action}:{Score}")]
public value struct ActionScore sealed
{
public:
  property uint32_t Action;

  property float Score;
};

/// <summary>
/// A action score/probability result.
/// </summary>
public ref class VowpalWabbitActionScoreBasePredictionFactory abstract
  : IVowpalWabbitPredictionFactory<cli::array<ActionScore>^>
{
public:
  /// <summary>
  /// Extracts multilabel prediction results from example.
  /// </summary>
  virtual cli::array<ActionScore>^ Create(vw* vw, example* ex) sealed;

  /// <summary>
  /// Returns the supported prediction type.
  /// </summary>
  property prediction_type_t PredictionType
  { virtual prediction_type_t get() abstract;
  }
};

/// <summary>
/// A action score prediction result.
/// </summary>
public ref class VowpalWabbitActionScorePredictionFactory sealed
    : public VowpalWabbitActionScoreBasePredictionFactory
{
public:
  /// <summary>
  /// Returns the supported prediction type.
  /// </summary>
  property prediction_type_t PredictionType
  { virtual prediction_type_t get() override sealed
    { return prediction_type_t::action_scores;
    }
  }
};

/// <summary>
/// A multi label prediction result.
/// </summary>
public ref class VowpalWabbitActionProbabilitiesPredictionFactory sealed
    : public VowpalWabbitActionScoreBasePredictionFactory
{
public:
  /// <summary>
  /// Returns the supported prediction type.
  /// </summary>
  property prediction_type_t PredictionType
  { virtual prediction_type_t get() override sealed
    { return prediction_type_t::action_probs;
    }
  }
};

/// <summary>
/// A topic prediction result.
/// </summary>
public ref class VowpalWabbitTopicPredictionFactory sealed : IVowpalWabbitPredictionFactory<cli::array<float>^>
{
public:
  /// <summary>
  /// Extracts prediction results from example. The predicted topics.
  /// </summary>
  virtual cli::array<float>^ Create(vw* vw, example* ex) sealed;

  /// <summary>
  /// Returns the supported prediction type.
  /// </summary>
  property prediction_type_t PredictionType
  { virtual prediction_type_t get() sealed
    { throw gcnew NotSupportedException("Prediction type is not available.");
    }
  }
};

/// <summary>
/// Provides convenient collection of all prediction types.
/// </summary>
public ref class VowpalWabbitPredictionType sealed abstract
{
public:
  /// <summary>
  /// Use for scalar predictions.
  /// </summary>
  static initonly VowpalWabbitScalarPredictionFactory^ Scalar = gcnew VowpalWabbitScalarPredictionFactory;

  /// <summary>
  /// Use for scalar predictions.
  /// </summary>
  static initonly VowpalWabbitScalarConfidencePredictionFactory^ ScalarConfidence = gcnew VowpalWabbitScalarConfidencePredictionFactory;

  /// <summary>
  /// Use for scalar predictions.
  /// </summary>
  static initonly VowpalWabbitScalarsPredictionFactory^ Scalars = gcnew VowpalWabbitScalarsPredictionFactory;

  /// <summary>
  /// Use for cost sensitive predictions.
  /// </summary>
  static initonly VowpalWabbitCostSensitivePredictionFactory^ CostSensitive = gcnew VowpalWabbitCostSensitivePredictionFactory;

  /// <summary>
  /// Use for multi label predictions.
  /// </summary>
  static initonly VowpalWabbitMultilabelPredictionFactory^ Multilabel = gcnew VowpalWabbitMultilabelPredictionFactory;

  /// <summary>
  /// Use for multi class predictions.
  /// </summary>
  static initonly VowpalWabbitMulticlassPredictionFactory^ Multiclass = gcnew VowpalWabbitMulticlassPredictionFactory;

  /// <summary>
  /// Use for action score predictions.
  /// </summary>
  static initonly VowpalWabbitActionScorePredictionFactory^ ActionScore = gcnew VowpalWabbitActionScorePredictionFactory;

  /// <summary>
  /// Use for action score predictions.
  /// </summary>
  static initonly VowpalWabbitActionProbabilitiesPredictionFactory^ ActionProbabilities = gcnew VowpalWabbitActionProbabilitiesPredictionFactory;

  /// <summary>
  /// Use for LDA topic predictions.
  /// </summary>
  static initonly VowpalWabbitTopicPredictionFactory^ Topic = gcnew VowpalWabbitTopicPredictionFactory;

  /// <summary>
  /// Use for dynamicially determined predictions.
  /// </summary>
  static initonly VowpalWabbitDynamicPredictionFactory^ Dynamic = gcnew VowpalWabbitDynamicPredictionFactory;

  /// <summary>
  /// Use for dynamicially determined predictions.
  /// </summary>
  static initonly VowpalWabbitProbabilityPredictionFactory^ Probability = gcnew VowpalWabbitProbabilityPredictionFactory;

  /// <summary>
  /// Use for multiclass predictions with probabilities
  /// </summary>
  static initonly VowpalWabbitMulticlassProbabilitiesPredictionFactory^ MultiClassProbabilities = gcnew VowpalWabbitMulticlassProbabilitiesPredictionFactory;
};
}

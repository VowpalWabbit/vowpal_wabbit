// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <msclr\marshal_cppstd.h>

using namespace System;
using namespace System::Collections::Generic;
using namespace System::IO;
using namespace System::Threading::Tasks;
using namespace VW::Serializer;

namespace VW
{
ref class VowpalWabbit;
ref class VowpalWabbitModel;
ref class VowpalWabbitSettings;

public enum class VowpalWabbitExampleDistribution
{
  /// <summary>
  /// Statistically safer option.
  /// </summary>
  UniformRandom = 0,

  /// <summary>
  /// Better runtime performance.
  /// </summary>
  RoundRobin = 1
};

public interface class ITypeInspector
{
public:
  Schema^ CreateSchema(VowpalWabbitSettings^ settings, Type^ type);
};

/// <summary>
/// Settings for wrapper.
/// </summary>
/// <remarks>Constructor with optional arguments was dropped as it broke version remapping (signature changed with the introduction of new options).</remarks>
public ref class VowpalWabbitSettings : public ICloneable
{
public:
  VowpalWabbitSettings()
  { Arguments = String::Empty;
    ExampleCountPerRun = 1000;
    MaxExampleCacheSize = UINT32_MAX;
    MaxExampleQueueLengthPerInstance = UINT32_MAX;
    EnableExampleCaching = false;
    // default to the statistically more safe option
    ExampleDistribution = VowpalWabbitExampleDistribution::UniformRandom;
    EnableStringExampleGeneration = false;
    EnableStringFloatCompact = false;
    PropertyConfiguration = ::PropertyConfiguration::Default;
    EnableThreadSafeExamplePooling = false;
    MaxExamples = INT32_MAX;
    Verbose = false;
  }

  VowpalWabbitSettings(String^ arguments)
    : VowpalWabbitSettings()
  { if (arguments != nullptr)
      Arguments = arguments;
  }

  /// <summary>
  /// Command line arguments.
  /// </summary>
  property String^ Arguments;

  /// <summary>
  /// Model used for initialization.
  /// </summary>
  property Stream^ ModelStream;

  /// <summary>
  /// Shared native vowpwal wabbit data structure.
  /// </summary>
  property VowpalWabbitModel^ Model;

  property ParallelOptions^ ParallelOptions;

  /// <summary>
  /// Set to true to disable example caching when used with a serializer. Defaults to true.
  /// </summary>
  property bool EnableExampleCaching;

  /// <summary>
  /// Maximum number of serialized examples cached. Defaults to UINT32_MAX.
  /// </summary>
  property uint32_t MaxExampleCacheSize;

  /// <summary>
  /// Maximum number of examples accepted by VowpalWabbitManager until Learn/Predict/... start to block. Defaults to UINT32_MAX.
  /// </summary>
  property uint32_t MaxExampleQueueLengthPerInstance;

  property uint32_t Node;

  property VowpalWabbit^ Root;

  property VowpalWabbitExampleDistribution ExampleDistribution;

  /// <summary>
  /// In multi-threaded mode, this is the number of examples processed per run.
  /// After ecah run the models are synchronized.
  /// Defaults to 1000.
  /// </summary>
  property uint32_t ExampleCountPerRun;

  /// <summary>
  /// Enable Vowpal Wabbit native string generation.
  /// </summary>
  property bool EnableStringExampleGeneration;

  /// <summary>
  /// Enable compact float serialization for Vowpal Wabbit native string generation.
  /// </summary>
  property bool EnableStringFloatCompact;

  property VW::Serializer::Schema^ Schema;

  property VW::Serializer::Schema^ ActionDependentSchema;

  property List<Type^>^ CustomFeaturizer;

  property ITypeInspector^ TypeInspector;

  property PropertyConfiguration^ PropertyConfiguration;

  property bool EnableThreadSafeExamplePooling;

  property int MaxExamples;

  property bool Verbose;

  /// <summary>
  /// Action invoked for each trace message.
  /// </summary>
  /// <Remarks>
  /// The trace listener obeys the Verbose property, which defaults to false.
  /// </Remarks>
  property Action<String^>^ TraceListener;

  virtual Object^ Clone()
  { return MemberwiseClone();
  }
};
}

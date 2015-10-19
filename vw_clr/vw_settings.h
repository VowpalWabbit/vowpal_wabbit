/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#pragma once

#include <msclr\marshal_cppstd.h>

using namespace System;
using namespace System::IO;
using namespace System::Threading::Tasks;

namespace VW
{
  ref class VowpalWabbitModel;
  ref class VowpalWabbit;

  public enum class VowpalWabbitExampleDistribution
  {
    // statistically safer option
    UniformRandom = 0,
    // better runtime performance
    RoundRobin = 1
  };

  public ref class VowpalWabbitSettings
  {
  private:
    String^ m_arguments;
    Stream^ m_modelStream;
    VowpalWabbitModel^ m_model;
    ParallelOptions^ m_parallelOptions;
    bool m_enableExampleCaching;
    uint32_t m_maxExampleCacheSize;
    uint32_t m_maxExampleQueueLengthPerInstance;
    uint32_t m_exampleCountPerRun;
    uint32_t m_node;
    VowpalWabbit^ m_root;
    VowpalWabbitExampleDistribution m_exampleDistribution;

  public:
    VowpalWabbitSettings() :
      m_arguments(String::Empty),
      m_exampleCountPerRun(1000),
      m_maxExampleCacheSize(UINT32_MAX),
      m_maxExampleQueueLengthPerInstance(UINT32_MAX),
      m_enableExampleCaching(true),
      // default to the statistically more safe option
      m_exampleDistribution(VowpalWabbitExampleDistribution::UniformRandom)
    {
    }

    VowpalWabbitSettings(String^ arguments)
      : VowpalWabbitSettings()
    {
      if (arguments != nullptr)
        m_arguments = arguments;
    }

    VowpalWabbitSettings([System::Runtime::InteropServices::Optional] String^ arguments,
      [System::Runtime::InteropServices::Optional] Stream^ modelStream,
      [System::Runtime::InteropServices::Optional] VowpalWabbitModel^ model,
      [System::Runtime::InteropServices::Optional] ParallelOptions^ parallelOptions,
      [System::Runtime::InteropServices::Optional] Nullable<bool> enableExampleCaching,
      [System::Runtime::InteropServices::Optional] Nullable<uint32_t> maxExampleCacheSize,
      [System::Runtime::InteropServices::Optional] Nullable<uint32_t> maxExampleQueueLengthPerInstance,
      [System::Runtime::InteropServices::Optional] Nullable<uint32_t> exampleCountPerRun,
      [System::Runtime::InteropServices::Optional] Nullable<uint32_t> node,
      [System::Runtime::InteropServices::Optional] VowpalWabbit^ root,
      [System::Runtime::InteropServices::Optional] Nullable<VowpalWabbitExampleDistribution> exampleDistribution)
      : VowpalWabbitSettings()
    {
      if (arguments != nullptr)
        m_arguments = arguments;

      m_model = model;
      m_modelStream = modelStream;
      m_parallelOptions = parallelOptions;
      m_root = root;

      if (enableExampleCaching.HasValue)
        m_enableExampleCaching = enableExampleCaching.Value;

      if (maxExampleCacheSize.HasValue)
        m_maxExampleCacheSize = maxExampleCacheSize.Value;

      if (exampleCountPerRun.HasValue)
        m_exampleCountPerRun = exampleCountPerRun.Value;

      if (maxExampleQueueLengthPerInstance.HasValue)
        m_maxExampleQueueLengthPerInstance = maxExampleQueueLengthPerInstance.Value;

      if (node.HasValue)
        m_node = node.Value;

      if (exampleDistribution.HasValue)
        m_exampleDistribution = exampleDistribution.Value;
    }

    /// <summary>
    /// Command line arguments.
    /// </summary>
    property String^ Arguments
    {
      String^ get()
      {
        return m_arguments;
      }
    }

    /// <summary>
    /// Model used for initialization.
    /// </summary>
    property Stream^ ModelStream
    {
      Stream^ get()
      {
        return m_modelStream;
      }
    }

    /// <summary>
    /// Shared native vowpwal wabbit data structure.
    /// </summary>
    property VowpalWabbitModel^ Model
    {
      VowpalWabbitModel^ get()
      {
        return m_model;
      }
    }

    property ParallelOptions^ ParallelOptions
    {
      System::Threading::Tasks::ParallelOptions^ get()
      {
        return m_parallelOptions;
      }
    }

    /// <summary>
    /// Set to true to disable example caching when used with a serializer. Defaults to true.
    /// </summary>
    property bool EnableExampleCaching
    {
      bool get()
      {
        return m_enableExampleCaching;
      }
    }

    /// <summary>
    /// Maximum number of serialized examples cached. Defaults to UINT32_MAX.
    /// </summary>
    property uint32_t MaxExampleCacheSize
    {
      uint32_t get()
      {
        return m_maxExampleCacheSize;;
      }
    }

    /// <summary>
    /// Maximum number of examples accepted by VowpalWabbitManager until Learn/Predict/... start to block. Defaults to UINT32_MAX.
    /// </summary>
    property uint32_t MaxExampleQueueLengthPerInstance
    {
      uint32_t get()
      {
        return m_maxExampleCacheSize;;
      }
    }

    property uint32_t Node
    {
      uint32_t get()
      {
        return m_node;
      }
    }

    property VowpalWabbit^ Root
    {
      VowpalWabbit^ get()
      {
        return m_root;
      }
    }

    property VowpalWabbitExampleDistribution ExampleDistribution
    {
      VowpalWabbitExampleDistribution get()
      {
        return m_exampleDistribution;
      }
    }

    /// <summary>
    /// In multi-threaded mode, this is the number of examples processed per run.
    /// After ecah run the models are synchronized.
    /// Defaults to 1000.
    /// </summary>
    property uint32_t ExampleCountPerRun
    {
      uint32_t get()
      {
        return m_exampleCountPerRun;
      }
    }

    VowpalWabbitSettings^ ShallowCopy(
      [System::Runtime::InteropServices::Optional] String^ arguments,
      [System::Runtime::InteropServices::Optional] Stream^ modelStream,
      [System::Runtime::InteropServices::Optional] VowpalWabbitModel^ model,
      [System::Runtime::InteropServices::Optional] System::Threading::Tasks::ParallelOptions^ parallelOptions,
      [System::Runtime::InteropServices::Optional] Nullable<bool> enableExampleCaching,
      [System::Runtime::InteropServices::Optional] Nullable<uint32_t> maxExampleCacheSize,
      [System::Runtime::InteropServices::Optional] Nullable<uint32_t> maxExampleQueueLengthPerInstance,
      [System::Runtime::InteropServices::Optional] Nullable<uint32_t> exampleCountPerRun,
      [System::Runtime::InteropServices::Optional] Nullable<uint32_t> node,
      [System::Runtime::InteropServices::Optional] VowpalWabbit^ root,
      [System::Runtime::InteropServices::Optional] Nullable<VowpalWabbitExampleDistribution> exampleDistribution)
    {
      auto copy = gcnew VowpalWabbitSettings();

      copy->m_model = model == nullptr ? Model : model;
      // don't copy arguments if model is set, as these are treated as extra arguments.
      if (arguments == nullptr)
      {
        if (copy->m_model == nullptr)
          copy->m_arguments = Arguments;
        else
          copy->m_arguments = String::Empty;
      }
      else
        copy->m_arguments = arguments;

      copy->m_modelStream = modelStream == nullptr ? ModelStream : modelStream;
      copy->m_parallelOptions = parallelOptions == nullptr ? ParallelOptions : parallelOptions;
      copy->m_enableExampleCaching = enableExampleCaching.HasValue ? enableExampleCaching.Value : EnableExampleCaching;
      copy->m_maxExampleCacheSize = maxExampleCacheSize.HasValue ? maxExampleCacheSize.Value : MaxExampleCacheSize;
      copy->m_maxExampleQueueLengthPerInstance = maxExampleQueueLengthPerInstance.HasValue ? maxExampleQueueLengthPerInstance.Value : MaxExampleQueueLengthPerInstance;
      copy->m_exampleCountPerRun = exampleCountPerRun.HasValue ? exampleCountPerRun.Value : ExampleCountPerRun;
      copy->m_node = node.HasValue ? node.Value : Node;
      copy->m_root = root == nullptr ? Root : root;
      copy->m_exampleDistribution = exampleDistribution.HasValue ? exampleDistribution.Value : ExampleDistribution;

      return copy;
    }
  };

}

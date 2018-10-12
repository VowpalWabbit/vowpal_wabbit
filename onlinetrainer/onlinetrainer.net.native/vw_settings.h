/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#pragma once
#include <stdint.h>

namespace online_trainer {

  enum class vw_example_distribution : short { UniformRandom, RoundRobin };

  class vw_model;
  class vw_settings {

  public:
    vw_settings();
    vw_settings(const char *args);

    const char *Arguments;
    int ExampleCountPerRun;
    uint32_t MaxExampleCacheSize;
    uint32_t MaxExampleQueueLengthPerInstance;
    bool EnableExampleCaching;
    vw_example_distribution ExampleDistribution;
    bool EnableStringExampleGeneration;
    bool EnableStringFloatCompact;
    // PropertyConfiguration
    bool EnableThreadSafeExamplePooling;
    int32_t MaxExamples;
    bool Verbose;
    vw_model *Model;
  };

}

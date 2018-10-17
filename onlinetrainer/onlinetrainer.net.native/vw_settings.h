/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#pragma once
#include <stdint.h>
#include <istream>
#include <streambuf>

namespace online_trainer {

  enum class vw_example_distribution : short { UniformRandom, RoundRobin };

  class vw_model;
  class membuf : public std::basic_streambuf<char> {
  public:
    membuf(const uint8_t *p, size_t l) {
      setg((char*)p, (char*)p, (char*)p + l);
    }
  };
  class memstream : public std::istream {
  public:
    memstream(const uint8_t *p, size_t l) : std::istream(&_buffer), _buffer(p, l) {
      rdbuf(&_buffer);
    }

  private:
    membuf _buffer;
  };

  class vw_settings {

  public:
    vw_settings()
    {
      ModelStream = nullptr;
      ExampleCountPerRun = 1000;
      MaxExampleCacheSize = UINT32_MAX;
      MaxExampleQueueLengthPerInstance = UINT32_MAX;
      EnableExampleCaching = false;
      ExampleDistribution = vw_example_distribution::UniformRandom;
      EnableStringExampleGeneration = false;
      EnableStringFloatCompact = false;
      //  PropertyConfiguration = ::PropertyConfiguration::Default;
      EnableThreadSafeExamplePooling = false;
      MaxExamples = INT32_MAX;
      Verbose = false;
    };

    vw_settings(const char * args) : vw_settings()
    {
      if (args != nullptr) {
        this->Arguments = args;
      }
    };

    const char *Arguments;
    memstream* ModelStream;
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

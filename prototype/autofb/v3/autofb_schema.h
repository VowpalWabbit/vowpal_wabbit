#pragma once

#include "base.h"

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/reflection.h"
#include "flatbuffers/reflection_generated.h"

namespace autofb 
{

struct bfbs_data
{
  std::string binary_data;
};

struct fbs_data
{
  std::string text_data;

  bfbs_data to_binary();
};



}
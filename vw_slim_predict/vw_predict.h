#pragma once

#include <memory>
#include <vector>
#include <string>

// avoid mmap dependency
#define DISABLE_SHARED_WEIGHTS
#include "array_parameters_dense.h"

#include "example_predict.h"

class vw_predict
{
  std::unique_ptr<dense_parameters> _weights;
  std::string _id;
  std::string _version;
  std::vector<std::string> _interactions;
  bool _ignore_linear[256];

public:
  vw_predict(const char* model, size_t length);

  void score(example_predict* shared, std::vector<example_predict*> actions, std::vector<float>& out_scores);
};

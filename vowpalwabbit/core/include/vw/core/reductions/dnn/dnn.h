// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/vw_fwd.h"
#include <memory>
#include <unordered_map>

// Forward declarations
namespace at {  class Tensor; }
namespace torch { namespace optim { class SGD; } }

namespace VW
{
namespace reductions
{
class hash_location_mapper
{
  public:
    hash_location_mapper(float* tensor_buffer, uint32_t num_inputs);
    at::Tensor tensor_from_example(const example& ec);
  private:
    uint64_t get_dense_location(uint64_t hash);
    std::unordered_map<uint64_t,uint64_t> _map_hash_to_dense;
    float* _tensor_buffer = nullptr;  // not owned
    uint32_t _num_inputs = 0;
};

struct Net;

class dnn_learner
{
public:
  void init(
    uint16_t num_layers,
    uint16_t hidden_layer_size,
    uint32_t num_inputs,
    float contraction /*How much to contract the prediction of dnn*/,
    uint32_t mini_batch_size);
  at::Tensor predict(example& ec);
  void learn(example& ec);
  void mini_batch_update();
  ~dnn_learner();
private:
  std::shared_ptr<hash_location_mapper> _phash_location_mapper;
  std::shared_ptr<Net> _pmodel;
  std::shared_ptr<torch::optim::SGD> _poptimizer;
  float* _tensor_buffer = nullptr;
  float _contraction = 1.0;         // How much to contract the prediction
  uint16_t _num_layers = 0;         // Number of layers in the network
  uint16_t _hidden_layer_size = 0;  // Size of the hidden layer
  uint32_t _num_initial_inputs = 0; // Number of inputs to the network
  uint32_t _mini_batch_size = 1;    // Number of examples to process in a batch
  uint32_t _accumulate_gradient_count = 0;
};


std::shared_ptr<VW::LEARNER::learner> dnn_setup(VW::setup_base_i& stack_builder);
}  // namespace reductions
}  // namespace VW

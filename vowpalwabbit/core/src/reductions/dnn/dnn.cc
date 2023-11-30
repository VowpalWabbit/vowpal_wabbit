#include "vw/core/reductions/dnn/dnn.h"
#include "vw/config/option_group_definition.h"
#include "vw/core/setup_base.h"
#include "vw/config/options.h"
#include "vw/core/learner.h"
#include <torch/torch.h>
#include <algorithm>

using namespace VW::LEARNER;
using namespace VW::config;

namespace VW { namespace reductions
{
struct Net : torch::nn::Module {
  Net(int16_t num_initial_inputs, int16_t hidden_layer_size, int32_t num_layers, uint32_t num_learners);
  using LinearPtr = std::shared_ptr<torch::nn::LinearImpl>;
  torch::Tensor forward(torch::Tensor x, uint32_t learner = 0);
  int32_t get_input_size();
  void increase_input_size(int64_t size);

private:
    std::vector<std::shared_ptr<torch::nn::LinearImpl>> _layers;
    std::vector<std::shared_ptr<torch::nn::LinearImpl>> _learners;
    int16_t _num_initial_inputs;
    int16_t _hidden_layer_size;
    int32_t _num_layers;
    uint32_t _num_learners;

    LinearPtr copy_and_expand(LinearPtr& old_layer, int64_t new_input_size);
};

Net::Net(int16_t num_initial_inputs, int16_t hidden_layer_size, int32_t num_layers, uint32_t num_learners):
  _num_initial_inputs(num_initial_inputs),
  _hidden_layer_size(hidden_layer_size),
  _num_layers(num_layers),
  _num_learners(num_learners) {
  // Construct and register n Linear submodules.
  // Save the module pointers in a vector.
  auto layer = register_module("fc0", torch::nn::Linear(_num_initial_inputs, _hidden_layer_size));
  _layers.push_back(layer);
  for(int i = 1; i < _num_layers-1; ++i) {
    auto layer = register_module("fc" + std::to_string(i), torch::nn::Linear(_hidden_layer_size, _hidden_layer_size));
    _layers.push_back(layer);
  }

  for(int i = 0; i < _num_learners; ++i) {
    // The last layer has only one output
    auto layer = register_module("fc_branch" + std::to_string(i), torch::nn::Linear(_hidden_layer_size, 1));
    _learners.push_back(layer);
  }
}

Net::LinearPtr Net::copy_and_expand(LinearPtr& old_layer, int64_t new_input_size) {
  using namespace torch::nn;
  int64_t old_input_size = old_layer->weight.size(1);
  int64_t old_output_size = old_layer->weight.size(0);
  // Create the new linear layer with the specified input and output sizes
  auto new_linear = std::make_shared<LinearImpl>(new_input_size, old_output_size);
  // Get the weights and biases from the old layer
  auto old_weights = old_layer->weight;
  auto old_biases = old_layer->bias;
  // create a serialized string with old size
  {
    // Copy the old parameters into the corresponding part of the new layer
    torch::NoGradGuard no_grad;
    new_linear->weight.slice(1, 0, old_input_size).copy_(old_weights);
    new_linear->bias.copy_(old_biases);
  }
  return new_linear;
}

torch::Tensor Net::forward(torch::Tensor x, uint32_t learner) {
  for(int i = 0; i < _layers.size() - 1; ++i ) {
    x = torch::relu(_layers[i]->forward(x));
  }
    
  // no relu on last layer
  x = _learners[learner]->forward(x);
  return x;
}

int32_t Net::get_input_size()
{
  return _layers[0]->weight.size(1);
}

void Net::increase_input_size(int64_t new_size)
{
    //If the input size is larger than the first layer, expand the layer
  if(new_size > _layers[0]->weight.size(1)) {
    _layers[0] = copy_and_expand(_layers[0], new_size);
  }
}

void dnn_learner::init(
    uint16_t num_layers,
    uint16_t hidden_layer_size,
    uint32_t num_inputs,
    float contraction,
    uint32_t mini_batch_size,
    uint32_t num_learners,
    bool debug_regression
 )
{
   std::cerr << "dnn_init(): num_layers: " << num_layers << ", hidden_layer_size: " << hidden_layer_size << ", num_inputs: " << num_inputs << ", num_learners: " << _num_learners << ", mini_batch_size: " << mini_batch_size << ", activation: relu, optimizer: SGD" << " contraction: " << contraction << std::endl;
  _contraction = contraction;
  _num_layers = num_layers;
  _hidden_layer_size = hidden_layer_size;
  _num_initial_inputs = num_inputs;
  _num_learners = num_learners;
  _debug_regression = debug_regression;
  _pmodel = std::make_shared<Net>(_num_initial_inputs, _hidden_layer_size, _num_layers, _num_learners);
  _poptimizer = std::shared_ptr<torch::optim::SGD>(new torch::optim::SGD(_pmodel->parameters(),0.01));
  _phash_location_mapper = std::make_shared<hash_location_mapper>(_num_initial_inputs);
}

std::string tensorToString(const at::Tensor& tensor) {
    std::ostringstream oss;
    for (int64_t i = 0; i < tensor.numel(); ++i) {
        oss << i << ":"<< tensor[i].item<float>();
        if (i != tensor.numel() - 1) {
            oss << " ";
        }
    }
    return oss.str();
}

at::Tensor dnn_learner::predict(const at::Tensor & input, uint32_t learner, bool debug_regression)
{
  if(debug_regression)
    std::cout << tensorToString(input);
  at::Tensor output = at::tensor(0.f);
  return _pmodel->forward(input, learner);
}

void dnn_learner::save_prediction(example& ec, float prediction)
{
  ec.partial_prediction = prediction;
  ec.partial_prediction *= static_cast<float>(_contraction);
  ec.pred.scalar = ec.partial_prediction;
}

at::Tensor dnn_learner::predict(example& ec, bool debug_regression)
{
  const at::Tensor input = _phash_location_mapper->tensor_from_example(ec);
  at::Tensor output = predict(input, ec.ft_offset, debug_regression);
  save_prediction(ec, output.item<float>());
  return output;
}

int64_t hash_location_mapper::get_dense_location(uint64_t hash, bool grow_buffer)
{
  const auto it = _map_hash_to_dense.find(hash);
  if (it == _map_hash_to_dense.end())
  {
    if(grow_buffer)
    {
      const uint64_t loc = _map_hash_to_dense.size();
      _map_hash_to_dense[hash] = loc;
      grow_tensor_buffer(loc+1);
      return loc;
    }
    else
    {
      return -1;
    }
  }
  return it->second;
}

hash_location_mapper::hash_location_mapper(uint32_t num_initial_inputs)
: _buffer_sz(num_initial_inputs), _tensor_buffer(new float[num_initial_inputs])
 {
    std::fill_n(_tensor_buffer, _buffer_sz, 0.0);
 }

at::Tensor hash_location_mapper::tensor_from_example(const example& ec, bool grow_buffer)
{
  auto& ep = (example_predict&)ec;

  // Zero out the tensor 
  std::fill_n(_tensor_buffer, _map_hash_to_dense.size(), 0.0);

  for (auto& ns : ep)
  {
    for (auto& n : ns)
    {
      const int64_t loc = get_dense_location(n.index(), grow_buffer);
      if(loc != -1)
        _tensor_buffer[loc] = n.value();
    }
  }

  return torch::from_blob(_tensor_buffer,_buffer_sz);
}

hash_location_mapper::~hash_location_mapper()
 {
   delete [] _tensor_buffer;
   _tensor_buffer = nullptr;
 }

uint32_t hash_location_mapper::tensor_growth_schedule(uint32_t min_size) const
{
   uint32_t proposal = 0; 
   // fast growth initially
   if (_buffer_sz <= 16)
   {
     proposal = _buffer_sz << 1; 
   }
   else // slow growth later
   {
     proposal = _buffer_sz + static_cast<uint32_t>(log(_buffer_sz) / log(2.0));
   }

   return (proposal > min_size)? proposal : min_size;
 }

void hash_location_mapper::grow_tensor_buffer(uint32_t desired_size)
 {
   if(desired_size <= _buffer_sz)
     return;
   const auto new_size = tensor_growth_schedule(desired_size);
   delete [] _tensor_buffer;
   _tensor_buffer = new float[new_size];
   _buffer_sz = new_size;
    std::fill_n(_tensor_buffer, _buffer_sz, 0.0);
 }

void dnn_learner::learn(example& ec)
{
  const at::Tensor input = _phash_location_mapper->tensor_from_example(ec, true);

  if(input.size(0) > _pmodel->get_input_size())
  {
    // force update of model parameters since
    // first layer is going to change
    mini_batch_update(true);
    _pmodel->increase_input_size(input.size(0));
  }

  if(_debug_regression)
    std::cout << ec.l.simple.label << " |a ";  // need the a so constant does not get a hash value of 0

  const auto output = predict(input, ec.ft_offset, _debug_regression);

  save_prediction(ec, output.item<float>());

  if(_debug_regression)
    std::cout << std::endl;

  auto y = torch::tensor({ec.l.simple.label});
  auto err = torch::mse_loss(output,y);
  ec.loss = err.item<float>();
  err.backward();
  ++_accumulate_gradient_count;
}

void dnn_learner::mini_batch_update(bool force)
{
  // accumulate gradients for mini_batch examples before updating parameters
  if(
      (force && _accumulate_gradient_count > 0) ||        // either force the update in the middle of a mini batch
      (_accumulate_gradient_count >= _mini_batch_size-1)  // or wait for the mini-batch to complete to update
    )
  {
    _poptimizer->step();
    _pmodel->zero_grad();
    _accumulate_gradient_count = 0;
  }
}

void predict(dnn_learner& dnn , VW::example& ec)
{
  dnn.predict(ec);
}

void learn(dnn_learner& dnn, VW::example& ec)
{
  dnn.learn(ec);
}

void update_stats(const VW::workspace& all, shared_data& sd, const dnn_learner&, const VW::example& ex, VW::io::logger& logger)
{
  sd.update(ex.test_only, ex.l.simple.is_labeled(), ex.loss, ex.weight, ex.get_num_features());
}

void cleanup_example(dnn_learner& dnn, VW::example&)
{
  dnn.mini_batch_update();
}

std::shared_ptr<learner> dnn_setup(VW::setup_base_i& stack_builder)
{
  config::options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();

  config::option_group_definition new_options("[Reduction] DNN");
  bool use_dnn = false;
  int num_inputs = 20;
  int num_layers = 3;
  int hidden_layer_size = 20;
  int mini_batch_size = 10;
  bool debug_regression = false;
  uint32_t num_learners = stack_builder.get_feature_width_above();

  new_options.add(make_option("dnn", use_dnn).keep().necessary().help("Fully connected deep neural network base learner."))
             .add(make_option("num_layers", num_layers).help("Number of Layers in the dnn"))
             .add(make_option("hidden_layer_size", hidden_layer_size).help("Size of hidden layers"))
             .add(make_option("debug_regression", debug_regression).help("Generate regression learning data in cerr"))
             .add(make_option("mini_batch_size", mini_batch_size).help("Number of gradients to accumulate before updating parameters."))
             .add(make_option("num_inputs", num_inputs).help("Size of the initial input vector to dnn"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  auto dnn = VW::make_unique<dnn_learner>();
  dnn->init(
    num_layers,
    hidden_layer_size,
    num_inputs,
    all.sd->contraction,
    mini_batch_size,
    num_learners,
    debug_regression
  );

  auto l = make_bottom_learner(
    std::move(dnn),learn, predict,
    stack_builder.get_setupfn_name(dnn_setup),
    prediction_type_t::SCALAR,label_type_t::SIMPLE)
     .set_cleanup_example(cleanup_example)
     .set_learn_returns_prediction(true)
     .set_update_stats(update_stats)
     .set_output_example_prediction(VW::details::output_example_prediction_simple_label<dnn_learner>)
     .set_print_update(VW::details::print_update_simple_label<dnn_learner>)
     .build();
  return l;
}
}  // namespace reductions
}  // namespace VW

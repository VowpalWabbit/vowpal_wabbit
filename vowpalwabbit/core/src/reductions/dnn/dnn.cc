#include "vw/core/reductions/dnn/dnn.h"
#include "vw/config/option_group_definition.h"
#include "vw/core/setup_base.h"
#include "vw/config/options.h"
#include "vw/core/learner.h"
#include <torch/torch.h>

using namespace VW::LEARNER;
using namespace VW::config;

namespace VW { namespace reductions
{
struct Net : torch::nn::Module {

  Net(int16_t num_initial_inputs, int16_t hidden_layer_size, int32_t num_layers, uint32_t num_learners) :
    _num_initial_inputs(num_initial_inputs),
    _hidden_layer_size(hidden_layer_size),
    _num_layers(num_layers),
    _num_learners(num_learners)
  {
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

  // Implement the Net's algorithm.
  torch::Tensor forward(torch::Tensor x, uint32_t learner = 0) {
    for(int i = 0; i < _layers.size() - 1; ++i ) {
        x = torch::relu(_layers[i]->forward(x));
    }
    
    // no relu on last layer
    x = _learners[learner]->forward(x);
    return x;
  }

  int32_t get_input_size() {
     return _num_initial_inputs;
   }

  private:
    std::vector<std::shared_ptr<torch::nn::LinearImpl>> _layers;
    std::vector<std::shared_ptr<torch::nn::LinearImpl>> _learners;
    int16_t _num_initial_inputs;
    int16_t _hidden_layer_size;
    int32_t _num_layers;
    uint32_t _num_learners;
};

 void dnn_learner::init(
    uint16_t num_layers,
    uint16_t hidden_layer_size,
    uint32_t num_inputs,
    float contraction,
    uint32_t mini_batch_size,
    uint32_t num_learners
 )
{
   std::cout << "dnn_init(): num_layers: " << num_layers << ", hidden_layer_size: " << hidden_layer_size << ", num_inputs: " << num_inputs << ", num_learners: " << _num_learners << ", mini_batch_size: " << mini_batch_size << ", activation: relu, optimizer: SGD" << " contraction: " << contraction << std::endl;
  _contraction = contraction;
  _num_layers = num_layers;
  _hidden_layer_size = hidden_layer_size;
  _num_initial_inputs = num_inputs;
  _num_learners = num_learners;
  _tensor_buffer = new float[_num_initial_inputs];
  _pmodel = std::make_shared<Net>(_num_initial_inputs, _hidden_layer_size, _num_layers, _num_learners);
  _poptimizer = std::shared_ptr<torch::optim::SGD>(new torch::optim::SGD(_pmodel->parameters(),0.01));
  _phash_location_mapper = std::make_shared<hash_location_mapper>(_tensor_buffer,_num_initial_inputs);
}

at::Tensor dnn_learner::predict(example& ec)
{
  //printf("dnn_predict()\n"); 
  at::Tensor input = _phash_location_mapper->tensor_from_example(ec);
  //std::cout << "input: \n" << input;
  at::Tensor output = _pmodel->forward(input, ec.ft_offset);

  //std::cout << "output: \n" << output;
  ec.partial_prediction = output.item<float>();
  //std::cout << "predicted: \n" << output;
  ec.partial_prediction *= static_cast<float>(_contraction);
  //std::cout << "contracted prediction: \n" << ec.partial_prediction;
  ec.pred.scalar = ec.partial_prediction;

  return output;
}

uint64_t hash_location_mapper::get_dense_location(uint64_t hash)
{
  auto it = _map_hash_to_dense.find(hash);
  if (it == _map_hash_to_dense.end()) {
    uint64_t loc = _map_hash_to_dense.size();
    _map_hash_to_dense[hash] = loc;
    return loc;
  }
  return it->second;
}

hash_location_mapper::hash_location_mapper(float* tensor_buffer, uint32_t num_inputs) :
  _tensor_buffer(tensor_buffer),
  _num_inputs(num_inputs)
{}

at::Tensor hash_location_mapper::tensor_from_example(const example& ec)
{
  example_predict& ep = (example_predict&)ec;
  float* dense = _tensor_buffer;
  for (VW::example_predict::iterator ns = ep.begin(); ns != ep.end(); ++ns) {
    for ( auto feat = (*ns).begin(); feat != (*ns).end(); ++feat) {
      uint64_t loc = get_dense_location((*feat).index());
      dense[loc] = (*feat).value();
    }
  }

  // Zero pad the rest of the tensor up to input_sz
  for (uint32_t i = _map_hash_to_dense.size(); i < _num_inputs; ++i) {
   dense[i] = 0.0;
  }
  return torch::from_blob(dense,_num_inputs);
}

void dnn_learner::learn(example& ec)
{
  //printf("dnn_learn()\n");
  auto output = predict(ec);
  auto y = torch::tensor({ec.l.simple.label});
  //std::cout << "expected: \n" << y;
  auto err = torch::mse_loss(output,y);
  ec.loss = err.item<float>();
  err.backward();
  //std::cout << "error: \n" << err;
  ++_accumulate_gradient_count;
}

dnn_learner::~dnn_learner() {
  delete[] _tensor_buffer;
}

void dnn_learner::mini_batch_update()
{
  // accumulate gradients for mini_batch examples before updating parameters
  if(_accumulate_gradient_count >= _mini_batch_size-1) {
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
  //printf("dnn_update_stats()\n");
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
  uint32_t num_learners = stack_builder.get_feature_width_above();

  new_options.add(make_option("dnn", use_dnn).keep().necessary().help("Fully connected deep neural network base learner."))
             .add(make_option("num_layers", num_layers).help("Number of Layers in the dnn"))
             .add(make_option("hidden_layer_size", hidden_layer_size).help("Size of hidden layers"))
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
    num_learners
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

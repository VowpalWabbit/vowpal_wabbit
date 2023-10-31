#include "vw/core/reductions/dnn/dnn.h"
#include "vw/config/option_group_definition.h"
#include "vw/core/setup_base.h"
#include "vw/config/options.h"
#include "vw/core/learner.h"
#include <torch/torch.h>

using namespace VW::LEARNER;
using namespace VW::config;

namespace VW
{
namespace reductions
{

void dnn_learner::init()
{
  torch::Tensor tensor = torch::rand({2,3});
  std::cout << "Hello from pytorch!" << tensor << std::endl;
}

void dnn_learner::predict(example& ec)
{
  printf("dnn_predict()\n"); 
}

void dnn_learner::learn(const example& ec)
{
  printf("dnn_learn()\n");   
}

void predict(dnn_learner& dnn , VW::example& ec)
{
  dnn.predict(ec);
}

void learn(dnn_learner& dnn, VW::example& ec)
{
  dnn.learn(ec);
}

void cleanup_example(dnn_learner& dnn, VW::example& ec)
{
  printf("dnn_cleanup_example()\n"); 
}

std::shared_ptr<learner> dnn_setup(VW::setup_base_i& stack_builder)
{
  config::options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();

  config::option_group_definition new_options("[Reduction] DNN");
  bool use_dnn = false;
  int num_inputs = 100;
  new_options.add(make_option("dnn", use_dnn).keep().necessary().help("Deep Neural Network Base Learner"))
             .add(make_option("num_inputs", num_inputs).help("Inputs to dnn"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  auto dnn = VW::make_unique<dnn_learner>();
  dnn->init();

  auto l = make_bottom_learner(
    std::move(dnn),learn, predict,
    stack_builder.get_setupfn_name(dnn_setup),
    prediction_type_t::SCALAR,label_type_t::SIMPLE)
     .set_learn_returns_prediction(true)
     .set_cleanup_example(nullptr)
     .build();
     // TODO: how do deal with multiple learners?
     // TODO: add the following functions
     // .set_multipredict(nullptr)
     // .set_update(nullptr)
     // .set_save_load(nullptr)
     // .set_end_pass(nullptr)
     // .set_merge_with_all(nullptr)
     // .set_add_with_all(nullptr)
     // .set_subtract_with_all(nullptr)
     // .set_output_example_prediction(nullptr)
     // .set_update_stats(nullptr)
     // .set_print_update(nullptr)
  return l;
}
}  // namespace reductions
}  // namespace VWW::reductions::dnn_setup(VW::setup_base_i& stack_builder)

 #include "vw/core/reductions/dnn/dnn.h"

 #include "vw/core/example.h"
 #include "vw/core/vw.h"
 #include "vw/test_common/test_common.h"
 #include <gmock/gmock.h>
 #include <gtest/gtest.h>
 #include <torch/torch.h>

 using namespace VW::reductions;


 class ExampleCreator {
 public:
   ExampleCreator() {
     _vw = VW::initialize(vwtest::make_args("--quiet", "--noconstant"));
   }

   std::shared_ptr<VW::example> create_example(std::string txt_example) {
     auto* ex = VW::read_example(*_vw, "1 |x a:2 |y b:3");
     return std::shared_ptr<VW::example>(ex,
       [this](VW::example* ex) { VW::finish_example(*_vw, *ex); });
   }

 private:
   std::shared_ptr<VW::workspace> _vw;
 };

 TEST(DNN, SmokeTest)
 {
   dnn_learner learner;
   learner.init(
     3, // num_layers
     2, // hidden_layer_size
     2, // num_inputs
     1.0, // prediction_contraction
     1, // mini_batch_size
     1, // num_learners
     false
   );

   ExampleCreator ex_creator;
   auto ec = ex_creator.create_example("1.0 |x f1:10 f2:20");
   learner.predict(*ec);
   learner.learn(*ec);
   ec = ex_creator.create_example("1.0 |x f3:10 f4:20");
   learner.learn(*ec);
   // Did the network expand?
   // Did the weights get copied over properly?
   // Is the prediction the same?
 }

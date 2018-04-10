#include "example_predict.h"
#include "array_parameters_dense.h"

enum Features
{
  // Shared
  Modality_Audio = 0,
  Modality_Video = 1,
  CallType_P2P = 2,
  CallType_Server = 3,
  NetworkType_Wired = 4,
  NetworkType_Wifi = 5,
  // ADF
  Action1 = 6,
  Action2 = 7,
  Action3 = 8
};

enum Namespaces
{
  SharedA = 0,
  SharedB = 1,
  SharedC = 2,
  ActionDependentX = 3,
  ActionDependentY = 4,
};

int main(int argc, char *argv[])
{
  // setup shared context
  safe_example_predict shared;

  shared.indices.push_back(SharedA);
  shared.indices.push_back(SharedB);
  shared.indices.push_back(SharedC);

  // 1-hot encoded feature
  shared.feature_space[SharedA].push_back(1, Features::Modality_Audio);
  shared.feature_space[SharedB].push_back(1, Features::CallType_P2P);
  shared.feature_space[SharedC].push_back(1, Features::NetworkType_Wired);

  // setup actions
  safe_example_predict action1, action2, action3;
  action1.indices.push_back(Namespaces::ActionDependentX);
  action2.indices.push_back(Namespaces::ActionDependentX);
  action3.indices.push_back(Namespaces::ActionDependentY);

  action1.feature_space[Namespaces::ActionDependentX].push_back(1, Features::Action1);
  action1.feature_space[Namespaces::ActionDependentX].push_back(1, Features::Action2);
  action1.feature_space[Namespaces::ActionDependentX].push_back(1, Features::Action3);

  // TODO: load model
  // TODO: slim parser
  // TODO: score
  /*
  inline_predict<dense_parameters>
    inline float inline_predict(W& weights, bool ignore_some_linear, bool ignore_linear[256], v_array<v_string>& interactions,
      bool permutations, example_predict& ec, float initial = 0.f)
      */

  return 0;
}

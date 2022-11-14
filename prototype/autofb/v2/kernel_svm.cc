#include "base.h"

#include "type_construtors.h"
#include "auto_flatbuf.h"
#include "vwtypes.h"

using namespace auto_flatbuf;
using namespace type_system;

namespace kernel_svm
{
// TC_DATA(flat_features)
// {
//   v_(uint64_t, indicies);
//   v_(float, values);
// };

// TC_DATA(flat_svm_example)
// {
//   _(float, simple_label); // TODO: We will want to support Unions as well

//   // reduction_features (which I do not believe are used?)

//   _(std::string, tag);
//   _(uint64_t, example_counter);
//   _(uint64_t, ft_offset);
//   _(float, global_weight);
//   _(uint64_t, num_features);
//   _(float, total_sum_feat_sq);
//   _(flat_features, fs);

//   DERIVE_REFLECT(flat_svm_example);
// };

// TC_DATA(svm_params_hyper)
// {
//   _(std::string, kernel_type);
//   _(float, bandwidth);
//   _(int, degree);
//   _(bool, para_active);
//   _(bool, active_pool_greedy);
//   _(uint64_t, pool_size); // these are not .keep()ed - why?
//   _(uint64_t, subsample);
//   _(uint64_t, reprocess);
// };

// TC_DATA(svm_params_predict)
// {
//   _(int32_t, num_support);
//   v_(flat_svm_example, support_data);
// };

// TC_DATA(svm_params_train)
// {
//   // this is an empty type?
//   // what does the serializer do when a type is not persisted because it is empty?
// };

struct svn_params_scratch
{

};

template <typename descriptor>
struct reduction_persisted
{
  typename descriptor::hyper_params_t hyper_params;
  typename descriptor::predict_params_t predict_params;
  typename descriptor::train_params_t train_params;
};

template <typename descriptor>
struct reduction_data
{
  typename descriptor::persisted_t persisted;
  typename descriptor::scratch_t scratch;
};

using reduction_configure_f = void(*)(pseudo_vw::stack_builder_t sb);
using reduction_predict_f = void(*)(pseudo_vw::example* ex);

struct kernel_svm_descriptor 
// this should really be a type-function, so we can just pass in the associated types (and names? should not be necessary at runtime)
// and it will generate the appropriate other type references and init functionality. The idea would be that it is possible to fill
// this in manually (and individually override each of the parts, but the "best-practice" is to just fill out the data types and 
// function pointers, and let the descriptor and associated factories actually implement the entry point)
//
// this should function, in part, like the means of, e.g. providing a custom hasher to std::unordered_map
//  in other words, reduction_init<descriptor_type>::setup(stack_builder_t sb)
//  or, alternatively, have the pointer sit directly on descriptor_type, and pull it out like:
//    typename T = descriptor_type
//    ...
//    T::setup_fn(stack_builder_t sb)
//
// the goal here is to enable reduction developers to work in a "typed" world, but have it be erased on the boundary to the rest of
// VW, thereby encapsulating the details of the reduction into the reduction itself. The only issue is that it does not allow for
// a reduction to add new label/prediction types, since the rest of VW needs to be aware of the set of type-classes that reductions
// can inhabit (because the label and prediction are available to code outside of it, e.g. polyprediction, polylabel, etc.).
{
  using type_class = struct regressor_typeclass
  {
    using example_t = pseudo_vw::example;
    using prediction_t = pseudo_vw::scalar_prediction;
    using label_t = pseudo_vw::simple_label;
  };
  
  // using hyper_params_t = svm_params_hyper;
  // using predict_params_t = svm_params_predict;
  // using train_params_t = svm_params_train;
  // using scratch_params_t = svn_params_scratch;

  using persisted_state_t = reduction_persisted<kernel_svm_descriptor>;
  using regressor_data_t = reduction_data<kernel_svm_descriptor>;
};

struct empty_t{}; // (alternatively void?)

void test_kernel_svm2()
{
  const std::string gen_dir = "C:/s/vw/vw_proto/prototype/autofb/generated/";
  schema_descriptor descriptor{"VW_proto"};

  auto_flatbuf::generate_universe_types(descriptor.schema_namespace, gen_dir);

  //serializer serializer(descriptor, gen_dir);

  // flat_svm_example example;
  // example.simple_label = 1.0f;
  // example.tag = "tag";
  // example.example_counter = 1;
  // example.ft_offset = 1;
  // example.global_weight = 1.0f;
  // example.num_features = 1;
  // example.total_sum_feat_sq = 1.0f;
  // example.fs().indicies.push_back(1);
  // example.fs().values.push_back(1.0f);

}

}
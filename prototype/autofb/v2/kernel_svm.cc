#include "base.h"

#include "type_construtors.h"
#include "auto_flatbuf.h"
#include "vwtypes.h"

#include "kernel_svm.h"

using namespace auto_flatbuf;
using namespace type_system;

namespace VW
{
namespace kernel_svm
{
TC_DATA(flat_features)
{
  v_(uint64_t, indicies);
  v_(float, values);
};

TC_DATA(flat_svm_example)
{
  _(float, simple_label);  // TODO: We will want to support Unions as well

  // reduction_features (which I do not believe are used?)

  _(std::string, tag);
  _(uint64_t, example_counter);
  _(uint64_t, ft_offset);
  _(float, global_weight);
  _(uint64_t, num_features);
  _(float, total_sum_feat_sq);
  _(flat_features, fs);

  // DERIVE_REFLECT(flat_svm_example);
};

TC_DATA(ksvm_config_p)
{
  _(std::string, kernel_type);
  _(float, bandwidth);
  _(int, degree);
  _(bool, para_active);
  _(bool, active_pool_greedy);
  _(uint64_t, pool_size);  // these are not .keep()ed - why?
  _(uint64_t, subsample);
  _(uint64_t, reprocess);
};

TC_DATA(ksvm_predict_p)
{
  _(int32_t, num_support);
  v_(flat_svm_example, support_data);
};

// TC_DATA(ksvm_learn_p)
//{
//   // this is an empty type?
//   // what does the serializer do when a type is not persisted because it is empty?
// };

namespace details
{
using RD = pseudo_vw::VW::REDUCTION::reduction_data<ksvm_config_p, ksvm_predict_p, pseudo_vw::VW::REDUCTION::empty_t>;
  
pseudo_vw::VW::LEARNER::base_learner* init_kernel_svm(
    pseudo_vw::VW::setup_base_i* stack_builder, const RD::config_params* config)
{
  return stack_builder->setup_base_learner();
}
}
}  // namespace kernel_svm
}  // namespace VW

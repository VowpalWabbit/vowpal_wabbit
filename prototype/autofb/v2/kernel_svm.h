#include "typed_reduction.h"

namespace VW
{

namespace kernel_svm
{
struct ksvm_config_p;
struct ksvm_predict_p;
using ksvm_learn_p = pseudo_vw::VW::REDUCTION::empty_t;

namespace details
{
extern const char kernel_svm_name[];
using RD = pseudo_vw::VW::REDUCTION::reduction_data<ksvm_config_p, ksvm_predict_p, ksvm_learn_p>;

pseudo_vw::VW::LEARNER::base_learner* init_kernel_svm(
    pseudo_vw::VW::setup_base_i* stack_builder, const RD::config_params* config);

using init_f = pseudo_vw::VW::REDUCTION::init_f_binder<RD, init_kernel_svm>;
}  // namespace details

using descriptor = pseudo_vw::VW::REDUCTION::reduction_descriptor<details::kernel_svm_name, details::init_f>;
}  // namespace kernel_svm
}  // namespace VW

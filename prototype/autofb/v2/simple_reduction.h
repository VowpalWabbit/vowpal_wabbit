#pragma once

#include "base.h"
#include "vwtypes.h"
#include "typed_reduction.h"

namespace VW
{
namespace binary
{
namespace details
{

extern const char binary_name[];

using RD = pseudo_vw::VW::REDUCTION::empty_reduction_data;

// Note that for reductions that do not have multiple versions, there is no need to bother with templating
template <uint32_t version>
pseudo_vw::VW::LEARNER::base_learner* init_binary(pseudo_vw::VW::setup_base_i* stack_builder, const RD::config_params* config)
{
  /*
  switch (version)
  {

  case 1:
    break;

  default:
  }
  */

  // actually pass the inner pointers and the rest of the configuration
  return stack_builder->setup_base_learner();
}

template <uint32_t version>
using init_f = pseudo_vw::VW::REDUCTION::init_f_binder<RD, init_binary<version>>;

}  // namespace details
   /// this is the export that "binary" needs to provide for the system to be able to consume it
template <uint32_t version = 1>
using descriptor = pseudo_vw::VW::REDUCTION::reduction_descriptor<details::binary_name, details::init_f<version>>;
}  // namespace binary
}

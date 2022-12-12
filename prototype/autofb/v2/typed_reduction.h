#pragma once

#include "base.h"
#include "vwtypes.h"

namespace pseudo_vw
{

namespace VW
{

namespace REDUCTION
{
struct empty_t
{
};

template <typename C, typename P, typename L>
struct reduction_data
{
  using config_params = C;
  using predict_params = P;
  using learn_params = L;
};

using empty_reduction_data = reduction_data<empty_t, empty_t, empty_t>;

template <typename C>
struct default_config
{
  bool operator()(pseudo_vw::VW::config::options_i* options, C** config)
  {
    erased_type t = type<C>::erase();
    // writing the rest in shorthand
    // activation a = (auto type_descriptor = GetTypeFromRegistry()) |> get_activator() |> activate();
    // bool configured = read_configuration_from_options(options, type_descriptor, t, a);
  }
};

template <typename RD, typename init_f>
struct check_descriptor : std::true_type
{
  // todo: validate that RD is actually a reduction_data
  // todo: validate that the type signature of init_f is correct viz. RD
};

template <typename RD = empty_reduction_data>
using typed_init_f = pseudo_vw::VW::LEARNER::base_learner* (*)(pseudo_vw::VW::setup_base_i*, const typename RD::config_params*);

template <typename RD, typed_init_f<RD> init_f = nullptr>
struct init_f_binder
{
  static_assert(init_f != nullptr, "must provide an initialization function");

  inline pseudo_vw::VW::LEARNER::base_learner* operator()(
      pseudo_vw::VW::setup_base_i* stack_builder, const typename RD::config_params* config)
  {
    return init_f(stack_builder, config);
  }
};

template <const char* name, typename init_f, const uint32_t version = 1, typename RD = empty_reduction_data,
    typename config_f = default_config<typename RD::config_params>, class = typename check_descriptor<RD, init_f>::type>
struct reduction_descriptor
{
  using reduction_data = RD;

  const char* name = name;
  const uint32_t version = version;

  using config = config_f;
  using init = init_f;
};
}  // namespace REDUCTION
}  // namespace VW
}  // namespace pseudo_vw

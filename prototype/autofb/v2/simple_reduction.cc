#include "simple_reduction.h"

#include "vwtypes.h"
#include "typed_reduction.h"

#include "type_construtors.h"
#include "type_erase.h"

namespace VW
{

template <typename descriptor_t>
struct registrator
{
  using RD = typename descriptor_t::reduction_data;

  using config_t = typename RD::config_params;

  inline static bool erased_config(pseudo_vw::VW::config::options_i* o, activation** target)
  {
    // target = GetTypeDescriptor<config_t> |> get_activator() |> activate();
    // config_t** c = get_ref_from_target(target);
    return descriptor_t::config(o, c);
  }

  inline static pseudo_vw::VW::LEARNER::base_learner* erased_init(
      pseudo_vw::VW::setup_base_i* stack_builder, erased_lvalue& erased_config)
  {
    // validate that we got the right config type
    if (!erased_config._type.is<config_t>())
    {
      // WARN("ERROR MESSAGE HERE");
      return nullptr;
    }

    return descriptor_t::init(stack_builder, &erased_config.get<config_t>());
  }

  inline static void invoke(void* reduction_stack)
  {
    // reduction_stack->enable_reduction(RD::name, RD::version, , RD::
  }
};

template <uint32_t target_version>
void* register_reductions()
{
  void* reduction_stack = nullptr;

  if (target_version > 1)
  {
    registrator<VW::binary::descriptor<1>>.invoke(reduction_stack);  // binary.1
  }

  if (target_version > 2)
  {
    // registrator<VW::binary::descriptor<2>>(); // binary.2
  }
}

}  // namespace VW

#pragma once

#include "../base.h"
#include "../vwtypes/vwtypes.h"
#include "../type_reflection.h"
#include "../type_activation.h"

#include "reduction_descriptor.h"

namespace VW
{
namespace t3_REDUCTION
{
struct options_i
{
  inline bool is_configured(pseudo_vw::option_group_definition group) { return false; }
};

struct registrator
{
  inline static void register_reduction(const reduction_descriptor& rd)
  {
    auto setup_f = [rd](options_i* o, typesys::activation** target) {
      const typesys::type_descriptor& td = *rd.reduction_data.config_params_td;
      
      // TODO: is this really what we want to do here?
      *target = std::move(&td.etype.activator());

      auto& config = **target;
      
      // for each in rd.options.options
      pseudo_vw::option_group_definition group(rd.name + std::string(" options"));
      for (auto& option_descriptor : rd.options)
      {
        //pseudo_vw::option_builder_t<> builder();
        //group.add(make_option(option_descriptor)) // this will require rewriting the option_builder_t
                                                    // to know how to deal with type erasure
        // use: 
          // option_descriptor.name
          // option_descriptor.aliases
          // to determine which options to interrogate inside of options_i
      }

      if (o->is_configured(group))
      {
        rd.init_f(o, config);
      }

      return false; // o.is_configured(group);
    };

    // reduction_stack->enable_reduction();
  }

private:
  void* reduction_stack;
};

template <uint32_t target_version>
void* register_reductions()
{
  void* reduction_stack = nullptr;

  if (target_version > 1)
  {
    //registrator<VW::binary::descriptor<1>>.invoke(reduction_stack);  // binary.1
    registrator.register_reduction(VW::simple::describe<1>());
  }

  if (target_version > 2)
  {
    // registrator<VW::binary::descriptor<2>>(); // binary.2
  }
}

} 
}
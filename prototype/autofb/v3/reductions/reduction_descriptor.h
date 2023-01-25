#pragma once

#include "../base.h"
#include "../vwtypes/vwtypes.h"

#include "../type_reflection.h"

namespace VW
{
  namespace t3_REDUCTION
  {
    struct reduction_data_descriptor
    {
      // we could use earsed_type here, but it would simply be used to look up the type_descriptor
      // so we might as well skip the dereference step (and the extra loose coupling)
      typesys::type_descriptor* config_params_td;
      typesys::type_descriptor* predict_params_td;
      typesys::type_descriptor* learn_params_td;
    };

    template <typename T>
    std::function<bool(typesys::erased_lvalue_ref&)> make_default_initializer(T value)
    {
      return [value](typesys::erased_lvalue_ref& ref) -> bool
      {
        if (!ref._type.is<T>())
        {
          return false;
        }
        else
        {
          ref.set<T>(value);
          return true;
        }
      }
    }

    const std::function<bool(typesys::erased_lvalue_ref&)> no_op = [](typesys::erased_lvalue_ref&) -> bool { return true; };

    struct option_descriptor
    {
      std::string name;
      std::string help;
      //std::vector<std::string> aliases; // TODO:
      typesys::property_descriptor pd;

      std::function<bool(typesys::erased_lvalue_ref&)> default_init_f = no_op;

      // TODO: deal with default in a type-erased manner
    };

    template <typename config_params>
    using typed_init_f = pseudo_vw::VW::LEARNER::base_learner* (*)(pseudo_vw::VW::setup_base_i*, const typename config_params*);

    using init_func_t = pseudo_vw::VW::LEARNER::base_learner*(pseudo_vw::VW::setup_base_i*, const typesys::activation&);

    template <typename config_params, typed_init_f<config_params> init_f = nullptr>
    std::function<init_func_t> erase_init_f()
    {
      static_assert(init_f != nullptr, "must provide an initialization function");

      return [](pseudo_vw::VW::setup_base_i* stack_builder, const typesys::activation& config)
      {
        return init_f(stack_builder, config.get<config_params>());
      }
    };

    struct reduction_descriptor
    {
      char* name;
      uint32_t version;

      // const label_t label_type
      // const predict_t prediction_type

      std::vector<option_descriptor> options;
      reduction_data_descriptor reduction_data;

      // In comparison to V2, the user is no longer responsible for specifying the config function,
      // because we can infer it from the type descriptor of the config_params, as well as the 
      // reduction options definition.
      // What I do not like about this is that we lose the type safety gained by compile-time checking
      // via the previous templated mechanism.

      //using config_func_t = bool(pseudo_vw::VW::config::options_i*, void**);
      //std::function<config_func_t> config_f;
      
      std::function<init_func_t> init_f;
    };
  };
}
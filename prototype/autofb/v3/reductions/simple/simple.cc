#include "simple.h"

#include "../../vwtypes/vwtypes.h"
#include "../../type_constructors.h"

template <typename V>
class option_builder
{
public:


private:
  typesys::property_descriptor _pd;
  V _default_value;

};



namespace VW
{
  namespace simple
  {
    namespace details_v1
    {
      using namespace VW::t3_REDUCTION;
      using namespace typesys;

      template <typename T, const char* name>
      struct type_descriptor_witness
      {
      public:
        type_descriptor_witness() : td(name, type<T>::erase())
        {
        }

        type_descriptor td;
      };

      ////

      struct empty_t
      {
      };

      extern const char simple_config_name[] = "simple_config";
      struct simple_config
      {
        Prop<int> a;
      };

      pseudo_vw::VW::LEARNER::base_learner* simple_init(pseudo_vw::VW::setup_base_i* stack_builder, const simple_config* config)
      {
        std::cout << "simple_init with " << config->a << std::endl;

        /*
        // exactly as currently done in setup()
        return make_learner().with_learn<simple_learn>().with_predict<simple_predict>();
        
        */

        return nullptr;
      }

      reduction_descriptor describe_v1()
      {
        erased_type etype = type<empty_t>::erase();
        type_descriptor td("empty_t", etype);

        erased_type config_etype = type<simple_config>::erase();
        type_descriptor config_td("simple_config", config_etype);

        // TODO: this is a bit of a hack for now, but these will come from the type system
        // in the future anyways (via TypeConstruction or manual binding)
        reduction_data_descriptor data { &config_td, &td, &td };

        // TODO: I really would like to get this inline with simple_config
        option_group<simple_config> simple_options("simple options");
        simple_options.bind<&simple_config::a>().essential().with_help("help for a in simple options");

        reduction_descriptor desc;
        desc.name = "simple";
        desc.version = 1;
        desc.reduction_data = data;
        desc.options = simple_options.get_options();

        desc.init_f = erase_init_f<simple_config, simple_init>();

        return desc;
      }
    }
  }
}
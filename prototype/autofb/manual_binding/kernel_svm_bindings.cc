#include "kernel_svm_bindings.h"

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/flatbuffer_builder.h"

// map ( type.name => "{type.name}_generated.h" )
#include "../generated/svm_params_hyper_generated.h"

#include "../typeerase.h"
#include "../typesys.h"
#include "../type_activation.h"

erased_data make_erased_data();

using namespace typesys;
using typesys::type_index;
//using typesys::type_info;

void test_roundtrip()
{
  erased_data data = make_erased_data();

  // this is CHEATING! (todo: this is available when building the type_info,
  //  so the serialization-side will be able to pack it (since reduction_builder
  //  will be defined in the .cc file))
  typesys::type_info& ti = universe::instance().get_type("svm_params_hyper", 1);

  std::for_each(ti.props_begin(), ti.props_end(),
    []
    (const property_info& pi)
    {
      
    });

  // note that we do not have access to the type here!

}
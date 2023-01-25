#include "../../base.h"
#include "../../vwtypes/vwtypes.h"

#include "../reduction_descriptor.h"

namespace VW
{
  namespace simple
  {
    namespace details_v1
    {
      t3_REDUCTION::reduction_descriptor describe_v1();
    }

    // this is here for illustrative purposes only; a reduction with only one version
    // should start by defining a non-templated describe() function
    template <uint32_t version>
    t3_REDUCTION::reduction_descriptor describe()
    {
      static_assert(version > 0 && version <= 1, "invalid version");

      switch (version)
      {
      case 1:
        return details_v1::describe_v1();
      default:
        // TODO: should we error out here? We have a check for it above in the static_assert
      }
    }
  }
}
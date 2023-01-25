#include "base.h"

#include "timpl_reduction_typeclass.h"

#pragma region Stuff that will go into reduction support?
using version_t = uint32_t;
#define __REDUCTION_VERSIONS_IMPL(min, max) \
namespace versions { \
  const version_t MIN_VERSION = min; \
  const version_t MAX_VERSION = max; \
}
#define REDUCTION_VERSIONS(min, max) __REDUCTION_VERSIONS_IMPL(min, max)
#define REDUCTION_VERSION(version) REDUCTION_VERSIONS(version, version)
struct reduction_options
{
public:
};
#pragma endregion

namespace t3
{
  REDUCTION_VERSIONS(1, 1);

  template <version_t version>
  struct descriptor
  {
    static_assert(version >= versions::MIN_VERSION && version <= versions::MAX_VERSION, "invalid version");

    template <typename = std::enable_if_t<version == 1>>
    static reduction_options get_options()
    {

    }


  };


}
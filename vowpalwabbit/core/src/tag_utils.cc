#include "vw/core/tag_utils.h"

#include "vw/core/example.h"

namespace VW
{
bool try_extract_random_seed(const example& ex, VW::string_view& view)
{
  if (!ex.tag.empty())
  {
    static const std::string SEED_IDENTIFIER = "seed=";
    const size_t prefix_length = SEED_IDENTIFIER.size();
    if (ex.tag.size() > prefix_length && strncmp(ex.tag.begin(), SEED_IDENTIFIER.c_str(), prefix_length) == 0)
    {
      view = VW::string_view(ex.tag.begin() + prefix_length, ex.tag.size() - prefix_length);
      return true;
    }
  }
  return false;
}
}  // namespace VW

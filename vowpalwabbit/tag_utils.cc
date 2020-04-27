#include "tag_utils.h"
#include "example.h"

namespace VW
{
  VW::string_view* extract_random_seed(const example& ex) {
    if (!ex.tag.empty())
    {
      const std::string SEED_IDENTIFIER = "seed=";
      if (strncmp(ex.tag.begin(), SEED_IDENTIFIER.c_str(), SEED_IDENTIFIER.size()) == 0 &&
          ex.tag.size() > SEED_IDENTIFIER.size())
      {
        return new VW::string_view(ex.tag.begin() + 5, ex.tag.size());
      }
    }
    return nullptr;
  }
}

#include "vw/core/cb.h"

namespace VW
{
class cb_with_observations_label
{
public:
  cb_label event;
  bool is_observation = false;
  bool is_definitely_bad = false;
};

bool ec_is_example_header_cb_with_observations(VW::example const& ec);

extern VW::label_parser cb_with_observations_global;
}  // namespace VW

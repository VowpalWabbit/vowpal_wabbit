#pragma once

#include "vw/core/cb.h"

namespace VW
{
class cb_with_observations_label
{
public:
  cb_label event;
  bool is_observation = false;
  bool is_definitely_bad = false;

  VW_ATTR(nodiscard) bool is_test_label() const;
  void reset_to_default();
};

bool ec_is_example_header_cb_with_observations(VW::example const& ec);

extern VW::label_parser cb_with_observations_global;
}  // namespace VW

namespace VW
{
namespace model_utils
{
size_t read_model_field(io_buf&, cb_with_observations_label&);
size_t write_model_field(io_buf&, const cb_with_observations_label&, const std::string&, bool);
}  // namespace model_utils
}  // namespace VW

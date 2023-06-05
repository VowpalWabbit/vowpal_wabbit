#include "vw/core/example.h"
#include "vw/core/model_utils.h"

namespace
{
float cb_with_observations_weight(const VW::cb_with_observations_label& ld) { return ld.event.weight; }

void parse_label_cb_with_observations(VW::cb_with_observations_label& /*ld*/, VW::reduction_features& /*red_features*/,
    VW::label_parser_reuse_mem& /*reuse_mem*/, const std::vector<VW::string_view>& /*words*/,
    VW::io::logger& /*logger*/)
{
  THROW("text format is not implemented for cb_with_observations label")
}
}  // namespace

namespace VW
{
namespace model_utils
{
size_t read_model_field(io_buf& io, VW::cb_with_observations_label& cb_with_obs)
{
  size_t bytes = 0;
  bytes += read_model_field(io, cb_with_obs.event);
  bytes += read_model_field(io, cb_with_obs.is_observation);
  bytes += read_model_field(io, cb_with_obs.is_definitely_bad);
  return bytes;
}

size_t write_model_field(
    io_buf& io, const VW::cb_with_observations_label& cb_with_obs, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += VW::model_utils::write_model_field(io, cb_with_obs.event, upstream_name + "_event", text);
  bytes += write_model_field(io, cb_with_obs.is_observation, upstream_name + "_is_observation", text);
  bytes += write_model_field(io, cb_with_obs.is_definitely_bad, upstream_name + "_is_definitely_bad", text);
  return bytes;
}
}  // namespace model_utils

bool ec_is_example_header_cb_with_observations(VW::example const& ec)
{
  const auto& costs = ec.l.cb_with_observations.event.costs;
  if (costs.size() != 1) { return false; }
  if (costs[0].probability == -1.f) { return true; }

  return false;
}

void cb_with_observations_label::reset_to_default()
{
  event.reset_to_default();
  is_observation = false;
  is_definitely_bad = false;
}

bool cb_with_observations_label::is_test_label() const { return event.is_test_label(); }

VW::label_parser cb_with_observations_global = {
    // default_label
    [](VW::polylabel& label) { label.cb_with_observations.reset_to_default(); },
    // parse_label
    [](VW::polylabel& label, VW::reduction_features& red_features, VW::label_parser_reuse_mem& reuse_mem,
        const VW::named_labels* /*ldict*/, const std::vector<VW::string_view>& words, VW::io::logger& logger)
    { parse_label_cb_with_observations(label.cb_with_observations, red_features, reuse_mem, words, logger); },
    // cache_label
    [](const VW::polylabel& label, const VW::reduction_features& /*red_features*/, io_buf& cache,
        const std::string& upstream_name, bool text)
    { return VW::model_utils::write_model_field(cache, label.cb_with_observations, upstream_name, text); },
    // read_cached_label
    [](VW::polylabel& label, VW::reduction_features& /*red_features*/, io_buf& cache)
    { return VW::model_utils::read_model_field(cache, label.cb_with_observations); },
    // get_weight
    [](const VW::polylabel& label, const VW::reduction_features& /*red_features*/)
    { return cb_with_observations_weight(label.cb_with_observations); },
    // test_label
    [](const VW::polylabel& label) { return label.cb_with_observations.is_test_label(); },
    // Label type
    VW::label_type_t::CB_WITH_OBSERVATIONS};
}  // namespace VW

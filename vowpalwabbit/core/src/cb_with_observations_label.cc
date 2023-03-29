#include "vw/core/example.h"

bool VW::ec_is_example_header_cb_with_observations(VW::example const& ec)
{
  const auto& costs = ec.l.cb_with_observations.event.costs;
  if (costs.size() != 1) { return false; }
  if (costs[0].probability == -1.f) { return true; }

  return false;
}

namespace
{
float weight_cb_with_observations(const VW::cb_with_observations_label& ld) { return ld.event.weight; }

void default_label_cb_with_observations(VW::cb_with_observations_label& ld)
{
  ld.event.reset_to_default();
  ld.is_observation = false;
  ld.is_definitely_bad = false;
}

bool test_label_cb_with_observations(const VW::cb_with_observations_label& ld) { return ld.event.is_test_label(); }

void parse_label_cb_with_observations(VW::cb_with_observations_label& /*ld*/, VW::reduction_features& /*red_features*/,
    VW::label_parser_reuse_mem& /*reuse_mem*/, const std::vector<VW::string_view>& /*words*/,
    VW::io::logger& /*logger*/)
{
  // TODO: implement text format parsing for cb with observations
}
} // namespace

VW::label_parser VW::cb_with_observations_global = {
    // default_label
    [](VW::polylabel& label) { default_label_cb_with_observations(label.cb_with_observations); },
    // parse_label
    [](VW::polylabel& label, VW::reduction_features& red_features, VW::label_parser_reuse_mem& reuse_mem,
        const VW::named_labels* /*ldict*/, const std::vector<VW::string_view>& words, VW::io::logger& logger)
    { parse_label_cb_with_observations(label.cb_with_observations, red_features, reuse_mem, words, logger); },
    // cache_label
    // TODO: implement this for cb_with_observations
    [](const VW::polylabel& label, const VW::reduction_features& /*red_features*/, io_buf& cache,
        const std::string& upstream_name, bool text)
    { return VW::model_utils::write_model_field(cache, label.cb, upstream_name, text); },
    // read_cached_label
    [](VW::polylabel& label, VW::reduction_features& /*red_features*/, io_buf& cache)
    { return VW::model_utils::read_model_field(cache, label.cb); },  // TODO: implement this for cb_with_observations
    // get_weight
    [](const VW::polylabel& label, const VW::reduction_features& /*red_features*/)
    { return weight_cb_with_observations(label.cb_with_observations); },
    // test_label
    [](const VW::polylabel& label) { return test_label_cb_with_observations(label.cb_with_observations); },
    // Label type
    VW::label_type_t::CB_WITH_OBSERVATIONS
};


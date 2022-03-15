// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "debug_log.h"
#include "learner.h"
#ifdef BUILD_EXTERNAL_PARSER
#  include "parse_example_external.h"
#endif
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>

#include <cfloat>

#include "config/options.h"
#include "global_data.h"
#include "io/logger.h"
#include "scope_exit.h"

using namespace VW::config;
using namespace VW::LEARNER;
using namespace rapidjson;

void insert_dsjson_metrics(
    const dsjson_metrics* ds_metrics, VW::metric_sink& metrics, const std::vector<std::string>& enabled_reductions)
{
  // ds_metrics is nullptr when --dsjson is disabled
  if (ds_metrics != nullptr)
  {
    metrics.set_uint("number_skipped_events", ds_metrics->NumberOfSkippedEvents);
    metrics.set_uint("number_events_zero_actions", ds_metrics->NumberOfEventsZeroActions);
    metrics.set_uint("line_parse_error", ds_metrics->LineParseError);
    metrics.set_string("first_event_id", ds_metrics->FirstEventId);
    metrics.set_string("first_event_time", ds_metrics->FirstEventTime);
    metrics.set_string("last_event_id", ds_metrics->LastEventId);
    metrics.set_string("last_event_time", ds_metrics->LastEventTime);
    metrics.set_float("dsjson_sum_cost_original", ds_metrics->DsjsonSumCostOriginal);
    if (std::find(enabled_reductions.begin(), enabled_reductions.end(), "ccb_explore_adf") != enabled_reductions.end())
    {
      metrics.set_float("dsjson_sum_cost_original_first_slot", ds_metrics->DsjsonSumCostOriginalFirstSlot);
      metrics.set_uint(
          "dsjson_number_label_equal_baseline_first_slot", ds_metrics->DsjsonNumberOfLabelEqualBaselineFirstSlot);
      metrics.set_uint("dsjson_number_label_not_equal_baseline_first_slot",
          ds_metrics->DsjsonNumberOfLabelNotEqualBaselineFirstSlot);
      metrics.set_float("dsjson_sum_cost_original_label_equal_baseline_first_slot",
          ds_metrics->DsjsonSumCostOriginalLabelEqualBaselineFirstSlot);
    }
    else
    {
      metrics.set_float("dsjson_sum_cost_original_baseline", ds_metrics->DsjsonSumCostOriginalBaseline);
    }
  }
}

namespace VW
{
namespace metrics
{
struct metrics_data
{
  std::string out_file;
  size_t learn_count = 0;
  size_t predict_count = 0;
};

struct json_metrics_writer : metric_sink_visitor
{
  json_metrics_writer(Writer<FileWriteStream>& writer) : _writer(writer) { _writer.StartObject(); }
  ~json_metrics_writer() override { _writer.EndObject(); }
  void int_metric(const std::string& key, uint64_t value) override
  {
    _writer.Key(key.c_str());
    _writer.Uint64(value);
  }
  void float_metric(const std::string& key, float value) override
  {
    _writer.Key(key.c_str());
    _writer.Double(static_cast<double>(value));
  }
  void string_metric(const std::string& key, const std::string& value) override
  {
    _writer.Key(key.c_str());

    _writer.String(value.c_str());
  }
  void bool_metric(const std::string& key, bool value) override
  {
    _writer.Key(key.c_str());
    _writer.Bool(value);
  }

private:
  Writer<FileWriteStream>& _writer;
};

void list_to_json_file(const std::string& filename, const metric_sink& metrics, VW::io::logger& logger)
{
  FILE* fp;
  if (VW::file_open(&fp, filename.c_str(), "wt") == 0)
  {
    auto file_closer = VW::scope_exit([fp]() { fclose(fp); });

    std::array<char, 1024> write_buffer;
    FileWriteStream os(fp, write_buffer.data(), write_buffer.size());
    Writer<FileWriteStream> writer(os);
    json_metrics_writer json_writer(writer);
    metrics.visit(json_writer);
  }
  else
  {
    logger.err_warn("skipping metrics. could not open file for metrics: {}", filename);
  }
}

void output_metrics(VW::workspace& all)
{
  if (all.options->was_supplied("extra_metrics"))
  {
    std::string filename = all.options->get_typed_option<std::string>("extra_metrics").value();
    metric_sink list_metrics;

    all.l->persist_metrics(list_metrics);

#ifdef BUILD_EXTERNAL_PARSER
    if (all.external_parser) { all.external_parser->persist_metrics(list_metrics); }
#endif

    list_metrics.set_uint("total_log_calls", all.logger.get_log_count());

    std::vector<std::string> enabled_reductions;
    if (all.l != nullptr) { all.l->get_enabled_reductions(enabled_reductions); }
    insert_dsjson_metrics(all.example_parser->metrics.get(), list_metrics, enabled_reductions);

    list_to_json_file(filename, list_metrics, all.logger);
  }
}

template <bool is_learn, typename T, typename E>
void predict_or_learn(metrics_data& data, T& base, E& ec)
{
  if (is_learn)
  {
    data.learn_count++;
    base.learn(ec);
  }
  else
  {
    data.predict_count++;
    base.predict(ec);
  }
}

void persist(metrics_data& data, metric_sink& metrics)
{
  metrics.set_uint("total_predict_calls", data.predict_count);
  metrics.set_uint("total_learn_calls", data.learn_count);
}

VW::LEARNER::base_learner* metrics_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  auto data = VW::make_unique<metrics_data>();

  option_group_definition new_options("[Reduction] Debug Metrics");
  new_options.add(make_option("extra_metrics", data->out_file)
                      .necessary()
                      .help("Specify filename to write metrics to. Note: There is no fixed schema"));

  if (!options.add_parse_and_check_necessary(new_options)) return nullptr;

  if (data->out_file.empty()) THROW("extra_metrics argument (output filename) is missing.");

  auto* base_learner = stack_builder.setup_base_learner();

  if (base_learner->is_multiline())
  {
    auto* l = make_reduction_learner(std::move(data), as_multiline(base_learner),
        predict_or_learn<true, multi_learner, multi_ex>, predict_or_learn<false, multi_learner, multi_ex>,
        stack_builder.get_setupfn_name(metrics_setup))
                  .set_output_prediction_type(base_learner->get_output_prediction_type())
                  .set_learn_returns_prediction(base_learner->learn_returns_prediction)
                  .set_persist_metrics(persist)
                  .build();
    return make_base(*l);
  }
  else
  {
    auto* l = make_reduction_learner(std::move(data), as_singleline(base_learner),
        predict_or_learn<true, single_learner, example>, predict_or_learn<false, single_learner, example>,
        stack_builder.get_setupfn_name(metrics_setup))
                  .set_output_prediction_type(base_learner->get_output_prediction_type())
                  .set_learn_returns_prediction(base_learner->learn_returns_prediction)
                  .set_persist_metrics(persist)
                  .build();
    return make_base(*l);
  }
}

}  // namespace metrics
}  // namespace VW

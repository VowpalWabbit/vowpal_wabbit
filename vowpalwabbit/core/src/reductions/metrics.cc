// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/metrics.h"

#include "vw/config/options.h"
#include "vw/core/crossplat_compat.h"
#include "vw/core/debug_log.h"
#include "vw/core/global_data.h"
#include "vw/core/learner.h"
#include "vw/core/parser.h"
#include "vw/core/scope_exit.h"
#include "vw/core/setup_base.h"
#include "vw/io/logger.h"

#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>

#include <cfloat>

using namespace VW::config;
using namespace VW::LEARNER;
using namespace rapidjson;
namespace
{
void insert_dsjson_metrics(const VW::details::dsjson_metrics* ds_metrics, VW::metric_sink& metrics,
    const std::vector<std::string>& enabled_learners)
{
  // ds_metrics is nullptr when --dsjson is disabled
  if (ds_metrics != nullptr)
  {
    metrics.set_uint("number_skipped_events", ds_metrics->number_of_skipped_events);
    metrics.set_uint("number_events_zero_actions", ds_metrics->number_of_events_zero_actions);
    metrics.set_uint("line_parse_error", ds_metrics->line_parse_error);
    metrics.set_string("first_event_id", ds_metrics->first_event_id);
    metrics.set_string("first_event_time", ds_metrics->first_event_time);
    metrics.set_string("last_event_id", ds_metrics->last_event_id);
    metrics.set_string("last_event_time", ds_metrics->last_event_time);
    metrics.set_float("dsjson_sum_cost_original", ds_metrics->dsjson_sum_cost_original);
    if (std::find(enabled_learners.begin(), enabled_learners.end(), "ccb_explore_adf") != enabled_learners.end())
    {
      metrics.set_float("dsjson_sum_cost_original_first_slot", ds_metrics->dsjson_sum_cost_original_first_slot);
      metrics.set_uint("dsjson_number_label_equal_baseline_first_slot",
          ds_metrics->dsjson_number_of_label_equal_baseline_first_slot);
      metrics.set_uint("dsjson_number_label_not_equal_baseline_first_slot",
          ds_metrics->dsjson_number_of_label_not_equal_baseline_first_slot);
      metrics.set_float("dsjson_sum_cost_original_label_equal_baseline_first_slot",
          ds_metrics->dsjson_sum_cost_original_label_equal_baseline_first_slot);
    }
    else { metrics.set_float("dsjson_sum_cost_original_baseline", ds_metrics->dsjson_sum_cost_original_baseline); }
  }
}

class metrics_data
{
public:
  size_t learn_count = 0;
  size_t predict_count = 0;
};

class json_metrics_writer : public VW::metric_sink_visitor
{
public:
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
  void sink_metric(const std::string& key, const VW::metric_sink& value) override
  {
    _writer.Key(key.c_str());
    _writer.StartObject();
    value.visit(*this);
    _writer.EndObject();
  }

private:
  Writer<FileWriteStream>& _writer;
};

void list_to_json_file(const std::string& filename, const VW::metric_sink& metrics, VW::io::logger& logger)
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
  else { logger.err_warn("skipping metrics. could not open file for metrics: {}", filename); }
}

void persist(metrics_data& data, VW::metric_sink& metrics)
{
  metrics.set_uint("total_predict_calls", data.predict_count);
  metrics.set_uint("total_learn_calls", data.learn_count);
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

void additional_metrics(VW::workspace& all, VW::metric_sink& sink)
{
  sink.set_uint("total_log_calls", all.logger.get_log_count());

  std::vector<std::string> enabled_learners;
  if (all.l != nullptr) { all.l->get_enabled_learners(enabled_learners); }
  insert_dsjson_metrics(all.example_parser->metrics.get(), sink, enabled_learners);
}
}  // namespace

void VW::reductions::output_metrics(VW::workspace& all)
{
  metrics_collector& manager = all.global_metrics;
  if (manager.are_metrics_enabled())
  {
    std::string filename = all.options->get_typed_option<std::string>("extra_metrics").value();
    list_to_json_file(filename, manager.collect_metrics(all.l.get()), all.logger);
  }
}

std::shared_ptr<VW::LEARNER::learner> VW::reductions::metrics_setup(VW::setup_base_i& stack_builder)
{
  VW::config::options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();

  auto data = VW::make_unique<metrics_data>();

  std::string out_file;
  option_group_definition new_options("[Reduction] Debug Metrics");
  new_options.add(make_option("extra_metrics", out_file)
                      .necessary()
                      .help("Specify filename to write metrics to. Note: There is no fixed schema"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  if (out_file.empty()) THROW("extra_metrics argument (output filename) is missing.");
  all.global_metrics = VW::metrics_collector(true);

  auto* all_ptr = stack_builder.get_all_pointer();
  all.global_metrics.register_metrics_callback(
      [all_ptr](VW::metric_sink& sink) -> void { additional_metrics(*all_ptr, sink); });

  auto base = stack_builder.setup_base_learner();

  if (base->is_multiline())
  {
    auto l = make_reduction_learner(std::move(data), require_multiline(base), predict_or_learn<true, learner, multi_ex>,
        predict_or_learn<false, learner, multi_ex>, stack_builder.get_setupfn_name(metrics_setup))
                 .set_output_prediction_type(base->get_output_prediction_type())
                 .set_learn_returns_prediction(base->learn_returns_prediction)
                 .set_persist_metrics(persist)
                 .build();
    return l;
  }
  else
  {
    auto l = make_reduction_learner(std::move(data), require_singleline(base), predict_or_learn<true, learner, example>,
        predict_or_learn<false, learner, example>, stack_builder.get_setupfn_name(metrics_setup))
                 .set_output_prediction_type(base->get_output_prediction_type())
                 .set_learn_returns_prediction(base->learn_returns_prediction)
                 .set_persist_metrics(persist)
                 .build();
    return l;
  }
}

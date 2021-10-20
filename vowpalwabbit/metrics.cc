// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "debug_log.h"
#include "reductions.h"
#include "learner.h"
#ifdef BUILD_EXTERNAL_PARSER
#  include "parse_example_external.h"
#endif
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>
#include <cfloat>

#include "io/logger.h"

using namespace VW::config;
using namespace VW::LEARNER;
using namespace rapidjson;

namespace logger = VW::io::logger;

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

void list_to_json_file(dsjson_metrics* ds_metrics, const std::string& filename, metric_sink& metrics,
    const std::vector<std::string>& enabled_reductions)
{
  FILE* fp;

  if (VW::file_open(&fp, filename.c_str(), "wt") == 0)
  {
    char writeBuffer[1024];
    FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
    Writer<FileWriteStream> writer(os);

    writer.StartObject();
    for (const auto& m : metrics.int_metrics_list)
    {
      writer.Key(m.first.c_str());
      writer.Uint64(m.second);
    }
    for (const auto& m : metrics.float_metrics_list)
    {
      writer.Key(m.first.c_str());
      writer.Double(static_cast<double>(m.second));
    }

    // ds_metrics is nullptr when --dsjson is disabled
    if (ds_metrics)
    {
      writer.Key("number_skipped_events");
      writer.Int64(ds_metrics->NumberOfSkippedEvents);
      writer.Key("number_events_zero_actions");
      writer.Int64(ds_metrics->NumberOfEventsZeroActions);
      writer.Key("line_parse_error");
      writer.Int64(ds_metrics->LineParseError);
      writer.Key("first_event_id");
      writer.String(ds_metrics->FirstEventId.c_str());
      writer.Key("first_event_time");
      writer.String(ds_metrics->FirstEventTime.c_str());
      writer.Key("last_event_id");
      writer.String(ds_metrics->LastEventId.c_str());
      writer.Key("last_event_time");
      writer.String(ds_metrics->LastEventTime.c_str());
      if (std::find(enabled_reductions.begin(), enabled_reductions.end(), "ccb_explore_adf") !=
          enabled_reductions.end())
      {
        writer.Key("dsjson_sum_cost_original_first_slot");
        writer.Double(ds_metrics->DsjsonSumCostOriginalFirstSlot);
        writer.Key("dsjson_number_label_equal_baseline_first_slot");
        writer.Int64(ds_metrics->DsjsonNumberOfLabelEqualBaselineFirstSlot);
        writer.Key("dsjson_number_label_not_equal_baseline_first_slot");
        writer.Int64(ds_metrics->DsjsonNumberOfLabelNotEqualBaselineFirstSlot);
        writer.Key("dsjson_sum_cost_original_label_equal_baseline_first_slot");
        writer.Double(ds_metrics->DsjsonSumCostOriginalLabelEqualBaselineFirstSlot);
      }
      else
      {
        writer.Key("dsjson_sum_cost_original_baseline");
        writer.Double(ds_metrics->DsjsonSumCostOriginalBaseline);
      }
      writer.Key("dsjson_sum_cost_original");
      writer.Double(ds_metrics->DsjsonSumCostOriginal);
    }

    writer.EndObject();

    fclose(fp);
  }
  else
  {
    logger::errlog_warn("skipping metrics. could not open file for metrics: {}", filename);
  }
}

void output_metrics(vw& all)
{
  if (all.options->was_supplied("extra_metrics"))
  {
    std::string filename = all.options->get_typed_option<std::string>("extra_metrics").value();
    metric_sink list_metrics;

    all.l->persist_metrics(list_metrics);

#ifdef BUILD_EXTERNAL_PARSER
    if (all.external_parser) { all.external_parser->persist_metrics(list_metrics.int_metrics_list); }
#endif

    list_metrics.int_metrics_list.emplace_back("total_log_calls", logger::get_log_count());

    std::vector<std::string> enabled_reductions;
    if (all.l != nullptr) { all.l->get_enabled_reductions(enabled_reductions); }

    list_to_json_file(all.example_parser->metrics.get(), filename, list_metrics, enabled_reductions);
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
  metrics.int_metrics_list.emplace_back("total_predict_calls", data.predict_count);
  metrics.int_metrics_list.emplace_back("total_learn_calls", data.learn_count);
}

VW::LEARNER::base_learner* metrics_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  auto data = VW::make_unique<metrics_data>();

  option_group_definition new_options("Debug: Metrics");
  new_options.add(make_option("extra_metrics", data->out_file)
                      .necessary()
                      .help("Specify filename to write metrics to. Note: There is no fixed schema."));

  if (!options.add_parse_and_check_necessary(new_options)) return nullptr;

  if (data->out_file.empty()) THROW("extra_metrics argument (output filename) is missing.");

  auto* base_learner = stack_builder.setup_base_learner();

  if (base_learner->is_multiline())
  {
    auto* l = make_reduction_learner(std::move(data), as_multiline(base_learner),
        predict_or_learn<true, multi_learner, multi_ex>, predict_or_learn<false, multi_learner, multi_ex>,
        stack_builder.get_setupfn_name(metrics_setup))
                  .set_prediction_type(base_learner->get_output_prediction_type())
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
                  .set_prediction_type(base_learner->get_output_prediction_type())
                  .set_learn_returns_prediction(base_learner->learn_returns_prediction)
                  .set_persist_metrics(persist)
                  .build();
    return make_base(*l);
  }
}

}  // namespace metrics
}  // namespace VW

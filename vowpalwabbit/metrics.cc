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
  size_t predicted_first_option = 0;
  size_t predicted_not_first = 0;
};

void list_to_json_file(std::string filename, std::vector<std::tuple<std::string, size_t>>& metrics)
{
  FILE* fp;

  if (VW::file_open(&fp, filename.c_str(), "wt") == 0)
  {
    char writeBuffer[1024];
    FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
    Writer<FileWriteStream> writer(os);

    writer.StartObject();
    for (std::tuple<std::string, size_t> m : metrics)
    {
      writer.Key(std::get<0>(m).c_str());
      writer.Int64(static_cast<int32_t>(std::get<1>(m)));
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
    std::vector<std::tuple<std::string, size_t>> list_metrics;

    all.l->persist_metrics(list_metrics);

#ifdef BUILD_EXTERNAL_PARSER
    if (all.external_parser)
    {
      all.external_parser->persist_metrics(list_metrics);
      // fetch metrics of parser
    }
#endif

    list_to_json_file(filename, list_metrics);
  }
}

void count_post_predict(metrics_data& data, prediction_type_t pred_type, example& ec)
{
  if (pred_type == prediction_type_t::multiclass)
  {
    if (ec.pred.multiclass == 1) { data.predicted_first_option++; }
    else
    {
      data.predicted_not_first++;
    }
  }
}

void count_post_predict(metrics_data& data, prediction_type_t pred_type, multi_ex& ec)
{
  if (pred_type == prediction_type_t::action_probs)
  {
    if (ec[0]->pred.a_s[0].action == 0) { data.predicted_first_option++; }
    else
    {
      data.predicted_not_first++;
    }
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

  if (!is_learn || base.learn_returns_prediction) { count_post_predict(data, base.pred_type, ec); }
}

void persist(metrics_data& data, std::vector<std::tuple<std::string, size_t>>& metrics)
{
  metrics.emplace_back("total_predict_calls", data.predict_count);
  metrics.emplace_back("total_learn_calls", data.learn_count);
  metrics.emplace_back("predicted_baseline_first", data.predicted_first_option);
  metrics.emplace_back("predicted_not_first", data.predicted_not_first);
}

VW::LEARNER::base_learner* metrics_setup(options_i& options, vw& all)
{
  auto data = scoped_calloc_or_throw<metrics_data>();

  option_group_definition new_options("Debug: Metrics");
  new_options.add(make_option("extra_metrics", data->out_file)
                      .necessary()
                      .help("Specify filename to write metrics to. Note: There is no fixed schema."));

  if (!options.add_parse_and_check_necessary(new_options)) return nullptr;

  if (data->out_file.empty()) THROW("extra_metrics argument (output filename) is missing.");

  auto* base_learner = setup_base(options, all);

  if (base_learner->is_multiline)
  {
    learner<metrics_data, multi_ex>* l = &init_learner(data, as_multiline(base_learner),
        predict_or_learn<true, multi_learner, multi_ex>, predict_or_learn<false, multi_learner, multi_ex>, 1,
        base_learner->pred_type, all.get_setupfn_name(metrics_setup), base_learner->learn_returns_prediction);
    l->set_persist_metrics(persist);
    return make_base(*l);
  }
  else
  {
    learner<metrics_data, example>* l = &init_learner(data, as_singleline(base_learner),
        predict_or_learn<true, single_learner, example>, predict_or_learn<false, single_learner, example>, 1,
        base_learner->pred_type, all.get_setupfn_name(metrics_setup), base_learner->learn_returns_prediction);
    l->set_persist_metrics(persist);
    return make_base(*l);
  }
}

}  // namespace metrics
}  // namespace VW

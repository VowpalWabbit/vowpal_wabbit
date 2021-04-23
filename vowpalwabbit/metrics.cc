// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "debug_log.h"
#include "reductions.h"
#include "learner.h"
#include "rapidjson/filewritestream.h"
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
  int learn_count = 0;
  int predict_count = 0;
  int predicted_first_option = 0;
  int predicted_not_first = 0;
};

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

void end_examples(metrics_data& data)
{
  std::cerr << "learn count: " << data.learn_count << std::endl;
  std::cerr << "predict count: " << data.predict_count << std::endl;
  std::cerr << "class one count: " << data.predicted_first_option << std::endl;
  std::cerr << "class not one count: " << data.predicted_not_first << std::endl;

  // where/when to write file?
  FILE* fp;

  if (VW::file_open(&fp, data.out_file.c_str(), "wt") == 0)
  {
    char writeBuffer[1024];
    FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
    Writer<FileWriteStream> writer(os);

    writer.StartObject();
    writer.Key("NumberOfPredicts");
    writer.Int(data.predict_count);
    writer.EndObject();

    fclose(fp);
  }
  else
  {
    logger::errlog_warn("warning: skipping metrics. could not open file for metrics: {}", data.out_file);
  }
}

VW::LEARNER::base_learner* metrics_setup(options_i& options, vw& all)
{
  auto data = scoped_calloc_or_throw<metrics_data>();

  option_group_definition new_options("Metrics");
  new_options.add(
      make_option("extra_metrics", data->out_file).necessary().help("persist extra metrics on specified filename"));

  if (!options.add_parse_and_check_necessary(new_options)) return nullptr;

  if (data->out_file.empty()) THROW("extra_metrics argument (output filename) is missing.");

  auto* base_learner = setup_base(options, all);

  if (base_learner->is_multiline)
  {
    learner<metrics_data, multi_ex>* l = &init_learner(data, as_multiline(base_learner),
        predict_or_learn<true, multi_learner, multi_ex>, predict_or_learn<false, multi_learner, multi_ex>, 1,
        prediction_type_t::action_probs, all.get_setupfn_name(metrics_setup), base_learner->learn_returns_prediction);
    l->set_end_examples(end_examples);
    return make_base(*l);
  }
  else
  {
    learner<metrics_data, example>* l = &init_learner(data, as_singleline(base_learner),
        predict_or_learn<true, single_learner, example>, predict_or_learn<false, single_learner, example>, 1,
        prediction_type_t::multiclass, all.get_setupfn_name(metrics_setup), base_learner->learn_returns_prediction);
    l->set_end_examples(end_examples);
    return make_base(*l);
  }
}

}  // namespace metrics
}  // namespace VW

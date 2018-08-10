#include "test_data_provider.h"

#include "utility/data_buffer.h"
#include "ranking_event.h"

#include <sstream>

test_data_provider::test_data_provider(const std::string& experiment_name, size_t threads, size_t examples, size_t features, size_t actions, bool _is_float_outcome)
  : event_ids(threads, std::vector<std::string>(examples))
  , contexts(threads, std::vector<std::string>(examples))
  , outcomes(threads, std::vector<std::string>(examples))
  , outcome_flag(threads, std::vector<bool>(examples))
  , is_float_outcome(_is_float_outcome)
{
  for (size_t t = 0; t < threads; ++t) {
    for (size_t i = 0; i < examples; ++i) {
      event_ids[t][i] = create_event_id(experiment_name, t, i);
      contexts[t][i] = create_context_json(create_features(features, t, i), create_action_features(actions, features, i));
      outcome_flag[t][i] = (i % 10 == 0);
      outcomes[t][i] = create_json_outcome(t, i);
    }
  }
}

std::string test_data_provider::create_event_id(const std::string& experiment_name, size_t thread_id, size_t example_id) const {
  std::ostringstream oss;
  oss << experiment_name << "-" << thread_id << "-" << example_id;
  return oss.str();
}

std::string test_data_provider::create_action_features(size_t actions, size_t features, size_t example_id) const {
  std::ostringstream oss;
  oss << R"("_multi": [ )";
  for (size_t a = 0; a < actions; ++a) {
	  oss << R"({ "TAction":{)";
	  for (size_t f = 0; f < features; ++f) {
      oss << R"("a_f_)" << f << R"(":"value_)" << (a + f + example_id) << R"(")";
      if (f + 1 < features) oss << ",";
	  }
    oss << "}}";
	  if (a + 1 < actions) oss << ",";
  }
  oss << R"(])";
  return oss.str();
}

std::string test_data_provider::create_features(size_t features, size_t thread_id, size_t example_id) const {
  std::ostringstream oss;
  oss << R"("GUser":{)";
  oss << R"("f_int":)" << example_id << R"(,)";
  oss << R"("f_float":)" << float(example_id) + 0.5 << R"(,)";
  for (size_t f = 0; f < features; ++f) {
    oss << R"("f_str_)" << f << R"(":"value_)" << (f + thread_id + example_id) << R"(")";
    if (f + 1 < features) oss << ",";
  }
  oss << R"(})";
  return oss.str();
}

std::string test_data_provider::create_json_outcome(size_t thread_id, size_t example_id) const {
  std::ostringstream oss;
  oss << R"({"Reward":)" << get_outcome(thread_id, example_id) << R"(,"CustomRewardField":)" << get_outcome(thread_id, example_id) + 1 << "}";
  return oss.str();
}

std::string test_data_provider::create_context_json(const std::string& cntxt, const std::string& action) const {
  std::ostringstream oss;
  oss << "{ " << cntxt << ", " << action << " }";
  return oss.str();
}

float test_data_provider::get_outcome(size_t thread_id, size_t example_id) const {
  return is_rewarded(thread_id, example_id) ? (thread_id  + example_id) : 0;
}

const char* test_data_provider::get_outcome_json(size_t thread_id, size_t example_id) const {
  return outcomes[thread_id][example_id].c_str();
}


const char* test_data_provider::get_event_id(size_t thread_id, size_t example_id) const {
  return event_ids[thread_id][example_id].c_str();
}

const char* test_data_provider::get_context(size_t thread_id, size_t example_id) const {
  return contexts[thread_id][example_id].c_str();
}

bool test_data_provider::is_rewarded(size_t thread_id, size_t example_id) const {
  return outcome_flag[thread_id][example_id];
}

void test_data_provider::log(size_t thread_id, size_t example_id, const reinforcement_learning::ranking_response& response, std::ostream& logger) const {
  size_t action_id;
  response.get_chosen_action_id(action_id);
  float prob = 0;
  for (auto it = response.begin(); it != response.end(); ++it) {
    if ((*it).action_id == action_id) {
      prob = (*it).probability;
    }
  }

  reinforcement_learning::utility::data_buffer buffer;
  logger << R"({"_label_cost":)" << -get_outcome(thread_id, example_id) << R"(,"_label_probability":)" << prob << R"(,"_label_Action":)" << (action_id + 1) << R"(,"_labelIndex":)" << action_id << ",";

  if (is_rewarded(thread_id, example_id)) {
    if (is_float_outcome)
      reinforcement_learning::outcome_event::serialize(buffer, get_event_id(thread_id, example_id), get_outcome(thread_id, example_id));
    else
      reinforcement_learning::outcome_event::serialize(buffer, get_event_id(thread_id, example_id), get_outcome_json(thread_id, example_id));

    logger << R"("o":[)" << buffer.str() << "],";
    buffer.reset();
  }

  reinforcement_learning::ranking_event::serialize(buffer, get_event_id(thread_id, example_id), get_context(thread_id, example_id), response);
  const std::string buffer_str = buffer.str();
  logger << buffer_str.substr(1, buffer_str.length() - 1) << std::endl;
}

int test_data_provider::report_outcome(reinforcement_learning::live_model* rl, size_t thread_id, size_t example_id, reinforcement_learning::api_status* status) const {
  if (is_float_outcome)
    return rl->report_outcome(get_event_id(thread_id, example_id), get_outcome(thread_id, example_id), status);
  return rl->report_outcome(get_event_id(thread_id, example_id), get_outcome_json(thread_id, example_id), status);
}
#pragma once

#include <vector>
#include <string>
#include <memory>

namespace ds {
  struct RankResponse {
    RankResponse(int action, std::vector<int>& ranking, const char* event_id,
      const char* model_version, std::vector<float>& probabilities, const char* features);
    // Note: each ActionProbability is yet another object requiring disposal...
    //std::vector<ActionProbability> ranking;

    const char* features;

    // top action == ranking[0]
    // added to safe allocation as ranking is allocated dynamically
    const int action;

    // actions order by our decision process
    // TODO: customs wig type. make RankResponse an iterable and return a small struct?
    const std::vector<int> ranking;

    // use boost::UUID + timestamp + additional data
    const std::string event_id;

    const std::string model_version;

    // parallel to ranking
    const std::vector<float> probabilities;
  };

  std::ostream& operator<<(std::ostream& ostr, const RankResponse& rankResponse);

  struct DecisionServiceConfiguration {
    static DecisionServiceConfiguration Download(const char* url);

    std::string model_url;

    std::string eventhub_interaction_connection_string;

    std::string eventhub_observation_connection_string;

    // defaults to 2
    int num_parallel_connection;

    // defaults to 5
    int batching_timeout_in_seconds;

    // defaults to 8*1024
    int batching_queue_max_size;

    // int pollingForModelPeriod;
    // int pollingForSettingsPeriod;
  };

  // avoid leakage to Swig
  class DecisionServiceClientInternal;

  // TODO: generat python doc: http://www.swig.org/Doc3.0/SWIGDocumentation.html#Python_nn65
  class DecisionServiceClient {
    std::unique_ptr<DecisionServiceClientInternal> _state;

  public:
    DecisionServiceClient(DecisionServiceConfiguration& config);

    ~DecisionServiceClient();

    // TODO: hand generate wrapper to get array size?
    RankResponse* rank(const char* features, const char* event_id, int* default_ranking, size_t default_ranking_size);

    void reward(const char* event_id, const char* reward);

    void update_model(unsigned char* model, size_t offset, size_t len);
  };
}

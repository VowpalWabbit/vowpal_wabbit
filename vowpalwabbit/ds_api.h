#pragma once

#include <vector>
#include <string>
#include <memory>

namespace ds {
  struct RankResponse {
    RankResponse(int action, std::vector<int>& ranking, const char* event_id, const char* model_version, std::vector<float>& probabilities);
    // Note: each ActionProbability is yet another object requiring disposal... 
    //std::vector<ActionProbability> ranking;
    
    // top action == ranking[0]
    // added to safe allocation as ranking is allocated dynamically
    const int action;

    // actions order by our decision process
    const std::vector<int> ranking;

    // use boost::UUID + timestamp + additional data
    const std::string event_id;

    const std::string model_version;
    
    // ordered by action id 1,2,3,...
    // TODO: probability lookup
    const std::vector<float> probabilities;
  };

  struct DecisionServiceConfiguration {
    // needs contain eventhub configuration
    std::string config_url;

    std::string model_url;

    std::string eventhub_connection_string;
    // int pollingForModelPeriod;
    // int pollingForSettingsPeriod;
  };

  class VowpalWabbitThreadSafe;

  class DecisionServiceClient {
    VowpalWabbitThreadSafe* _pool;
  public:
    DecisionServiceClient(DecisionServiceConfiguration config);
    ~DecisionServiceClient();

    // TODO: automatic generation of event id?
    //RankResponse* rank(RankRequest& request);
    RankResponse* rank(const char* data, const char* event_id, int* default_ranking, size_t default_ranking_size);

    // TODO: observation
    void reward(const char* event_id, const char* reward);

    void update_model(unsigned char* model, size_t offset, size_t len);
  };
}
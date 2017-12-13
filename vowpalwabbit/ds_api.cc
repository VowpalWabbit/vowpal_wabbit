#include "ds_api.h"
#include "ds_internal.h"
#include "ds_vw.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>


namespace ds {
  //RankRequest::RankRequest(const char* pdata, const char* pevent_id)
  //  : data(pdata), event_id(pevent_id)
  //{ }
  
  RankResponse::RankResponse(int paction, std::vector<int>& pranking, const char* pevent_id, const char* pmodel_version, std::vector<float>& pprobabilities)
    : action(paction), ranking(pranking), event_id(pevent_id), model_version(pmodel_version), probabilities(pprobabilities)
  { }

  DecisionServiceClient::DecisionServiceClient(DecisionServiceConfiguration config)
    :  _pool(new VowpalWabbitThreadSafe())
  {
    // initialize back threads
    // - model download
    // - event upload
  }

  DecisionServiceClient::~DecisionServiceClient() {
    if (_pool) {
      delete _pool;
      _pool = nullptr;
    }
  }

  RankResponse* DecisionServiceClient::rank(const char* context, const char* event_id, int* default_ranking, size_t default_ranking_size)
  {
    // generate event id if empty
    std::string l_event_id;
    if (!event_id || strlen(event_id) == 0)
      l_event_id = boost::uuids::to_string(boost::uuids::random_generator()());
    else
      l_event_id = event_id;

    // invoke scoring to get a distribution over actions
    std::vector<ActionProbability> ranking = _pool->rank(context);
    
    // invoke sampling
    // need to port random generator
    // need to port top slot explortation

    return nullptr; //new RankResponse();
  }

  void DecisionServiceClient::reward(const char* event_id, const char* reward)
  {
    
    // enqueue reward for upload
  }

  void DecisionServiceClient::update_model(unsigned char* model, size_t offset, size_t len)
  {
    // swap out model
  }
}
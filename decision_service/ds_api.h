/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#pragma once

#include <vector>
#include <string>
#include <memory>

#ifndef DISABLE_NAMESPACE
namespace Microsoft {
  namespace DecisionService {
#endif
    class RankResponse {
      // actions order by our decision process
      const std::vector<int> _ranking;

      // parallel to ranking
      const std::vector<float> _probabilities;

      const char* features;

    public:
#ifndef SWIG
      RankResponse(std::vector<int>& ranking, const char* event_id,
        const char* model_version, std::vector<float>& probabilities, const char* features);
#endif

      // use boost::UUID + timestamp + additional data
      const std::string event_id;

      const std::string model_version;

      int top_action();

      const std::vector<float>& probabilities();

      const std::vector<int>& ranking();

#ifndef ARRAYS_OPTIMIZED
      int length();

      int action(int index);

      float probability(int index);
#endif

#ifndef SWIG
      friend std::ostream& operator<<(std::ostream& ostr, const RankResponse& rankResponse);
#endif
    };

#ifndef SWIG
    std::ostream& operator<<(std::ostream& ostr, const RankResponse& rankResponse);
#endif

    // helper type to avoid memory copy for C#
    template<typename T>
    struct Array
    {
      T* data;
      size_t length;
    };


    enum DecisionServiceLogLevel {
      none = 0,
      error = 1,
      warning = 2,
      trace = 4
    };

    class DecisionServiceLogger {
    public:
      virtual ~DecisionServiceLogger() { }

      virtual void log(DecisionServiceLogLevel level, const std::string& message) { }

      // TODO: add more events (e.g. model download)
    };

    class DecisionServiceConfiguration {

    public:
      static DecisionServiceConfiguration Download(const char* url/*, bool certificate_validation_enabled = true*/) throw(std::exception);

      DecisionServiceConfiguration();

      std::string model_url;

      std::string eventhub_interaction_connection_string;

      std::string eventhub_observation_connection_string;

      bool certificate_validation_enabled;

      // defaults to 2
      int num_parallel_connection;

      // defaults to 5sec
      int batching_timeout_in_milliseconds;

      // defaults to 8*1024
      int batching_queue_max_size;

      // int pollingForModelPeriod;
      // int pollingForSettingsPeriod;

      // defaults to error
      DecisionServiceLogLevel log_level;

      // TODO: understand how memory ownership works...
      DecisionServiceLogger* logger;

#ifndef SWIG
      bool can_log(DecisionServiceLogLevel log_level);
#endif
    };

    class DecisionServicePredictionResult {
      public:
        void set(const std::vector<float>& score);
        void set(const Array<float>& default_ranking);
        void set(const float* default_ranking, size_t default_ranking_size);
    };

    // doesn't work with feature modifying reductions
    // this bears some threading/timing issues:
    // the closure will have to get a "version" lock 
    // to make sure the same version is used through out the calls.
    class DecisionServicePredictionIterator {
    public:
      virtual ~DecisionServicePredictionIterator() { }

      // unclear how to solve memory handoff, or just copy?
      // do a custom object, though this might be a pain to copy, at least it will involve memory copy
      // result.set([....]);
      virtual bool next_prediction(const std::vector<int>& previous_decisions, DecisionServicePredictionResult* result) { return false; }
      // TODO: look into swig
      // virtual bool next_prediction(const Array<int>& previous_decisions, DecisionServicePredictionResult* result) { return false; }

      // don't pass the context back to avoid memory copy
      // virtual std::vector<float> predict() { }
    };

    class DecisionServicePredictionIteratorSimple {
      public:
        // int and float
        DecisionServicePredictionIteratorSimple(std::vector<float>& scores);
    };

    // ordered by actionId, pass numActions to understand numActions & numModels
    // for CCB make the assumption that subsequent rounds make the same decision
    // float[] scoresMatrix; // int length;

    // avoid leakage to Swig
    class DecisionServiceClientInternal;

    // TODO: generat python doc: http://www.swig.org/Doc3.0/SWIGDocumentation.html#Python_nn65
    class DecisionServiceClient {
      std::unique_ptr<DecisionServiceClientInternal> _state;

    public:
      DecisionServiceClient(DecisionServiceConfiguration& config);

      ~DecisionServiceClient();

      // named rank1, 2, 3,... to ease ignore/rename matching in swig

      RankResponse* rank_cstyle(const char* features, const char* event_id, const int* default_ranking, size_t default_ranking_size);

      RankResponse* rank_struct(const char* features, const char* event_id, const Array<int>& default_ranking);

      RankResponse* rank_vector(const char* features, const char* event_id, const std::vector<int>& default_ranking);

      // ClientLibrary cl;
      // model m1;
      // model m2;
      // cl.rank("...", [m1, m2], 2);
      // { DecisionServicePredictionResult r1; m1[0]->predict_ccb(&r1); }
      //
      void rank(const char* features, DecisionServicePredictionIterator* model_iterator);
      
      // cl.rank("...", [scores]);
      // cl.rank("...", lambda _, out: out.set([scores]); return false;)  
      // cl.rank("...", DecisionServicePredictionIteratorSimple([scores]))

      void reward(const char* event_id, const char* reward);

      // TODO: drop this. 
      void update_model(unsigned char* model, size_t offset, size_t len);

      void update_model(unsigned char* model, size_t len);
    };

#ifndef SWIG
#define DS_LOG(config, level, msg) { \
  if (config.can_log(level)) { \
    std::ostringstream __message; \
    __message << msg; \
    config.logger->log(level, __message.str()); \
  } }
#endif

#ifndef DISABLE_NAMESPACE
  }
}
#endif

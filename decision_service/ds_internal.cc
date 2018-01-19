#include "ds_internal.h"
#include "ds_vw.h"
#include "ds_predictor_container.h"

using namespace std;
#include "utility.h"

#include <chrono>

namespace Microsoft {
  namespace DecisionService {
    using namespace utility;
    using namespace web::http;
    using namespace web::http::client;
    using namespace concurrency;
    using namespace concurrency::streams;
    using namespace std;

    DecisionServiceClientInternal::DecisionServiceClientInternal(DecisionServiceConfiguration config)
        : //  _pool(new VowpalWabbitThreadSafe()),
        _config(config),
        _queue(config.batching_queue_max_size),
        _thread_running(true),
        // initialize n-clients
        _event_hub_interactions(config.num_parallel_connection, EventHubClient(config.eventhub_interaction_connection_string, config.certificate_validation_enabled)),
        _event_hub_observation(config.eventhub_observation_connection_string, config.certificate_validation_enabled),
        // TODO: daemon mode?
        _upload_interaction_thread(&DecisionServiceClientInternal::upload_interactions, this),
        _download_model_thread(&DecisionServiceClientInternal::download_model, this),
        _default_explorer(0.2f)
      {
          // pre-compute application has for salting the seed
        _seed_from_app_id = MultiWorldTesting::HashUtils::Compute_Id_Hash(config.app_id);

        if (!_config.explorer)
          _config.explorer = &_default_explorer;
      }

      DecisionServiceClientInternal::~DecisionServiceClientInternal()
      {
        _thread_running = false;

        // wait for the threads to finish, don't delete before completion
        _upload_interaction_thread.join();
        _download_model_thread.join();

        // delete all elements from _queue
        vector<unsigned char>* item;
        while (_queue.pop(item))
          delete item;
      }

      void DecisionServiceClientInternal::upload_reward(const char* event_id, const char* reward)
      {
        ostringstream payload;
        payload << "{\"EventId\":\"" << event_id << "\",\"v\":" << reward << "}";

        _event_hub_observation.Send(payload.str().c_str());
      }

      void DecisionServiceClientInternal::enqueue_interaction(RankResponse& rankResponse)
      {
        std::ostringstream ostr;
        ostr << rankResponse;
        auto s = ostr.str();

        vector<unsigned char>* buffer = new vector<unsigned char>(s.begin(), s.end());
        _queue.push(buffer);
      }

      void DecisionServiceClientInternal::download_model()
      {
        // TODO: download using cpprest
      }

      void DecisionServiceClientInternal::upload_interactions()
      {
        vector<unsigned char>* json;
        vector<unsigned char>* json2;

        // populate initial already finished tasks
        vector<pplx::task<void>> open_requests;

        http_response initial_response;
        initial_response.set_status_code(201);

        for (auto& client : _event_hub_interactions)
        {
          pplx::task_completion_event<void> evt;
          evt.set();
          open_requests.push_back(pplx::task<void>(evt));
        }

        while (_thread_running)
        {
          if (!_queue.pop(json))
          {
            // empty
            // cout << "queue empty" << endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(_config.batching_timeout_in_milliseconds));

            continue;
          }

          int batch_count = 1;

          do
          {
            if (!_queue.pop(json2))
              json2 = nullptr;
            else
            {
              // check if we enough space
              if (json->size() + json2->size() < 256 * 1024 - 1)
              {
                // move get pointer to the beginning
                json->push_back('\n');
                json->insert(json->end(), json2->begin(), json2->end());

                batch_count++;

                // free memory
                delete json2;

                // accumulate more
                continue;
              }
            }

            // manage multiple outstanding requests...
            // send data
            auto ready_idx = pplx::when_any(open_requests.begin(), open_requests.end()).get();

            //printf("ready: %d length: %d batch count: %d status: %d\n",
            //  (int)ready_response.second, (int)json_str.length(), batch_count, ready_response.first.status_code());

            // send request and add back to task list
            open_requests[ready_idx] =
              _event_hub_interactions[ready_idx]
              .Send(json)
              .then([=](pplx::task<http_response> resp) {
              try
              {
                auto http_resp = resp.get();
                if (http_resp.status_code() != 201)
                  DS_LOG(_config, DecisionServiceLogLevel::error, "Failed to upload event: '" << http_resp.status_code() << "'")
              }
              catch (std::exception& e)
              {
                DS_LOG(_config, DecisionServiceLogLevel::error, "Failed to upload event: '" << e.what() << "'")
              }
            });

            // continue draining if we have another non-append json element
            json = json2;
            batch_count = 1;
          } while (json);
        }
      }
  }
}
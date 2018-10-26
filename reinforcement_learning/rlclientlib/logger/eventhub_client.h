#pragma once

#include "api_status.h"
#include "moving_queue.h"
#include "sender.h"
#include <vector>
#include "error_callback_fn.h"

#include <cpprest/http_client.h>
#include <pplx/pplxtasks.h>

#include <queue>

namespace reinforcement_learning {
  class i_trace;

  //the eventhub_client send string data in POST request to an http endpoint
  //it handles authorization headers specific for the azure event hubs
  class eventhub_client : public i_sender {
  public:
    virtual int init(api_status* status) override;

    eventhub_client(const std::string& host, const std::string& key_name,
                    const std::string& key, const std::string& name,
                    size_t tasks_count, i_trace* trace, error_callback_fn* _error_cb, bool local_test = false);
    ~eventhub_client();
  protected:
    virtual int v_send(std::vector<unsigned char> &&data, api_status* status) override;

  private:
    class http_request_task {
    public:
      http_request_task();
      http_request_task(web::http::client::http_client& client, const std::string& host, const std::string& auth, const std::vector<unsigned char> &&data, error_callback_fn* _error_cb);
      http_request_task(http_request_task&& other);
      http_request_task& operator=(http_request_task&& other);

      web::http::status_code join();
      std::vector<unsigned char> post_data() const;
    private:
      http_request_task(const http_request_task&) = delete;
      http_request_task& operator=(const http_request_task&) = delete;

    private:
      std::vector<unsigned char> _post_data;
      pplx::task<web::http::status_code> _task;
    };

  private:
    int authorization(api_status* status);
    int submit_task(http_request_task&& task, api_status* status);
    int pop_task(api_status* status);

    // cannot be copied or assigned
    eventhub_client(const eventhub_client&) = delete;
    eventhub_client(eventhub_client&&) = delete;
    eventhub_client& operator=(const eventhub_client&) = delete;
    eventhub_client& operator=(eventhub_client&&) = delete;

  private:
    web::http::client::http_client _client;

    const std::string _eventhub_host; //e.g. "ingest-x2bw4dlnkv63q.servicebus.windows.net"
    const std::string _shared_access_key_name; //e.g. "RootManageSharedAccessKey"
    const std::string _shared_access_key;
    //e.g. Check https://docs.microsoft.com/en-us/azure/event-hubs/event-hubs-authentication-and-security-model-overview
    const std::string _eventhub_name; //e.g. "interaction"

    std::string _authorization;
    long long _authorization_valid_until; //in seconds
    std::mutex _mutex;
    moving_queue<http_request_task> _tasks;
    const size_t _max_tasks_count;
    i_trace* _trace;
    error_callback_fn* _error_callback;
  };
}

#pragma once

#include "api_status.h"
#include "moving_queue.h"
#include "sender.h"
#include "error_callback_fn.h"

#include <cpprest/http_client.h>
#include <pplx/pplxtasks.h>

#include <queue>
#include <vector>
#include <chrono>
#include <memory>

namespace reinforcement_learning {
  class i_trace;

  // The eventhub_client send string data in POST requests to an HTTP endpoint.
  // It handles authorization headers specific for the Azure event hubs.
  class eventhub_client : public i_sender {
  public:
    virtual int init(api_status* status) override;

    eventhub_client(const std::string& host, const std::string& key_name,
      const std::string& key, const std::string& name,
      size_t tasks_count, size_t MAX_RETRIES, i_trace* trace, error_callback_fn* _error_cb, bool local_test = false);
    ~eventhub_client();
  protected:
    virtual int v_send(std::vector<unsigned char>&& data, api_status* status) override;

  private:
    class http_request_task {
    public:
      http_request_task() = default;
      http_request_task(
        web::http::client::http_client* client,
        const std::string& host,
        const std::string& auth,
        std::vector<unsigned char>&& data,
        size_t max_retries = 1, // If MAX_RETRIES is set to 1, only the initial request will be attempted.
        error_callback_fn* error_callback = nullptr,
        i_trace* trace = nullptr);

      // The constructor kicks off an async request which captures the this variable. If this object is moved then the
      // this pointer is invalidated and causes tricky bugs.
      http_request_task(http_request_task&& other) = delete;
      http_request_task& operator=(http_request_task&& other) = delete;
      http_request_task(const http_request_task&) = delete;
      http_request_task& operator=(const http_request_task&) = delete;

      std::vector<unsigned char> post_data() const;

      // Return error_code
      int join();
    private:
      pplx::task<web::http::status_code> send_request(size_t try_count);

      web::http::client::http_client* _client;
      std::string _host;
      std::string _auth;
      std::vector<unsigned char> _post_data;

      pplx::task<web::http::status_code> _task;

      size_t _max_retries = 1;

      error_callback_fn* _error_callback;
      i_trace* _trace;
    };

  private:
    int check_authorization_validity_generate_if_needed(api_status* status);

    static int generate_authorization_string(
      std::chrono::seconds now,
      const std::string& shared_access_key,
      const std::string& shared_access_key_name,
      const std::string& eventhub_host,
      const std::string& eventhub_name,
      std::string& authorization_string /* out */,
      long long& valid_until /* out */,
      api_status* status,
      i_trace* trace);

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
    std::mutex _mutex_http_tasks;
    moving_queue<std::unique_ptr<http_request_task>> _tasks;
    const size_t _max_tasks_count;
    const size_t _max_retries;
    i_trace* _trace;
    error_callback_fn* _error_callback;
  };
}

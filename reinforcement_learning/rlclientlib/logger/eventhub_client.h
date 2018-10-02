#pragma once

#include "api_status.h"
#include "moving_queue.h"
#include "sender.h"

#include <cpprest/http_client.h>
#include <pplx/pplxtasks.h>

#include <queue>

namespace reinforcement_learning {

  //the eventhub_client send string data in POST request to an http endpoint
  //it handles authorization headers specific for the azure event hubs
  class eventhub_client : public i_sender {
  public:
    virtual int init(api_status* status) override;

    eventhub_client(const std::string&, const std::string&,
                    const std::string&, const std::string&,
                    size_t tasks_count, bool local_test = false);
    ~eventhub_client();

  protected:
    virtual int v_send(std::string&& data, api_status* status) override;

  private:
    class task {
    public:
      task();
      task(web::http::client::http_client& client, const std::string& host, const std::string& auth, std::string&& post_data);
      task(task&& other);
      task& operator=(task&& other);

      web::http::status_code join();
      std::string post_data() const;
    private:
      task(const task&) = delete;
      task& operator=(const task&) = delete;

    private:
      std::string _post_data;
      pplx::task<web::http::status_code> _task;
    };

  private:
    int authorization(api_status* status);
    int submit_task(task&& task, api_status* status);
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
    moving_queue<task> _tasks;
    const size_t _tasks_count;
  };
}

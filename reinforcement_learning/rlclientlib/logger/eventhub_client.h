#pragma once

#include "api_status.h"
#include "logger.h"

#include <cpprest/http_client.h>

namespace reinforcement_learning {

  //the eventhub_client send string data in POST request to an http endpoint
  //it handles authorization headers specific for the azure event hubs
  class eventhub_client : public i_logger {
  public:
    virtual int init(api_status* status) override;

    //send a POST request
    int send(const std::string&, api_status* status = nullptr);

    eventhub_client(const std::string&, const std::string&,
                    const std::string&, const std::string&, bool local_test = false);

  protected:
    virtual int v_append(const std::string& data, api_status* status) override;

  private:
    int authorization(api_status* status);
    web::http::client::http_client _client;

    const std::string _eventhub_host; //e.g. "ingest-x2bw4dlnkv63q.servicebus.windows.net"
    const std::string _shared_access_key_name; //e.g. "RootManageSharedAccessKey"
    const std::string _shared_access_key;
    //e.g. Check https://docs.microsoft.com/en-us/azure/event-hubs/event-hubs-authentication-and-security-model-overview
    const std::string _eventhub_name; //e.g. "interaction"

    std::string _authorization;
    long long _authorization_valid_until; //in seconds
    std::mutex _mutex;

    // cannot be copied or assigned
    eventhub_client(const eventhub_client&) = delete;
    eventhub_client(eventhub_client&&) = delete;
    eventhub_client& operator=(const eventhub_client&) = delete;
    eventhub_client& operator=(eventhub_client&&) = delete;
  };
}

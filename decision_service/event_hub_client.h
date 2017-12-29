#pragma once

#include <string>
#include <memory>
#include <istream>
#include <cpprest/http_client.h>

namespace ds {

  class EventHubClient
  {
  private:
    // must be a pointer to enable initialization at the end of ctor
    std::unique_ptr<web::http::client::http_client> _client;
    std::string _connection_string;

    std::string _event_hub_namespace_host;
    std::string _event_hub;
    std::string _key_name;
    std::string _key;
    std::string _authorization;
    // in seconds from epoch
    long long _authorization_valid_until;

    std::string& Authorization();

  public:
    EventHubClient(std::string connection_string);

    EventHubClient(const EventHubClient& other);

    pplx::task<web::http::http_response> Send(const char* data);

    pplx::task<web::http::http_response> EventHubClient::Send(std::vector<unsigned char>* data);
  };
}

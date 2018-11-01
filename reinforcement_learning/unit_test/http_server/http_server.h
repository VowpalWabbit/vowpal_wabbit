#pragma once
#include <cpprest/http_listener.h>

#include <functional>

using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

class http_server {
public:
  using response_fn = void(const http_request&);

  http_server(const bool post_err = false)
    : _post_err(post_err) {}

  http_server(const utility::string_t& url, const bool posterr = false);

  pplx::task<void> open() {
    return m_listener.open();
  }

  pplx::task<void> close() {
    return m_listener.close();
  }

  void set_post_error(bool value);
  void set_custom_responder(http::method, std::function<response_fn> custom_responder);
private:
  void handle_get(const http_request& message);
  void handle_put(const http_request& message);
  void handle_post(const http_request& message);
  void handle_delete(const http_request& message);
  void handle_head(const http_request& message);

  http_listener m_listener;
  bool _post_err;

  std::map<http::method, std::function<response_fn>> _custom_responders;
};

class http_helper {
  std::unique_ptr<http_server> g_http;

public:
  bool on_initialize(const string_t& address, const bool post_error = false) {
    try {
      // Build our listener's URI from the configured address and the hard-coded path "MyServer/Action"
      uri_builder uri(address);
      const auto addr = uri.to_uri().to_string();
      g_http = std::unique_ptr<http_server>(new http_server(addr, post_error));
      return (g_http->open().wait() == pplx::completed);
    }
    catch (const std::exception&) {
      return false;
    }
  }

  void set_post_error(bool value) {
    g_http->set_post_error(value);
  }

  void set_custom_responder(http::method method, std::function<http_server::response_fn> custom_responder) {
    g_http->set_custom_responder(method, custom_responder);
  }

  ~http_helper() {
    if (g_http != nullptr) {
      g_http->close().wait();
    }
  }
};

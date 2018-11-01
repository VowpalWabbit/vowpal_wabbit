#include "stdafx.h"
#include "http_server.h"

using namespace std;
using namespace web;
using namespace utility;
using namespace http;
using namespace web::http::experimental::listener;

http_server::http_server(const utility::string_t& url, const bool post_err)
  : m_listener(url), _post_err(post_err) {
  m_listener.support(methods::GET, std::bind(&http_server::handle_get, this, std::placeholders::_1));
  m_listener.support(methods::PUT, std::bind(&http_server::handle_put, this, std::placeholders::_1));
  m_listener.support(methods::POST, std::bind(&http_server::handle_post, this, std::placeholders::_1));
  m_listener.support(methods::DEL, std::bind(&http_server::handle_delete, this, std::placeholders::_1));
  m_listener.support(methods::HEAD, std::bind(&http_server::handle_head, this, std::placeholders::_1));
}

void http_server::set_post_error(bool value) {
  _post_err = value;
}

void http_server::set_custom_responder(http::method method, std::function<response_fn> custom_responder) {
  _custom_responders[method] = custom_responder;
}

void http_server::handle_get(const http_request& message) {
  if (_custom_responders.count(message.method()) != 0) {
    _custom_responders[message.method()](message);
    return;
  }

  http_response resp;
  resp.set_status_code(status_codes::OK);
  resp.headers().add(U("Last-Modified"), datetime::utc_now().to_string());
  resp.set_body(U("Http GET response"));
  message.reply(resp).get();
}

void http_server::handle_post(const http_request& message) {
  if (_custom_responders.count(message.method()) != 0) {
    _custom_responders[message.method()](message);
    return;
  }

  if (_post_err)
    message.reply(status_codes::InternalError);
  else {
    message.reply(status_codes::Created);
  }
}

void http_server::handle_head(const http_request& message) {
  if (_custom_responders.count(message.method()) != 0) {
    _custom_responders[message.method()](message);
    return;
  }

  http_response resp;
  resp.set_status_code(status_codes::OK);
  resp.headers().add(U("Last-Modified"), datetime::utc_now().to_string());
  resp.set_body(U("Http HEAD response"));
  message.reply(resp).get();
}

void http_server::handle_delete(const http_request& message) {
  if (_custom_responders.count(message.method()) != 0) {
    _custom_responders[message.method()](message);
    return;
  }

  message.reply(status_codes::OK);
}

void http_server::handle_put(const http_request& message) {
  if (_custom_responders.count(message.method()) != 0) {
    _custom_responders[message.method()](message);
    return;
  }

  message.reply(status_codes::OK);
}

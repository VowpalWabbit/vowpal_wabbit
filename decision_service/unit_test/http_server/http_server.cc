#include "stdafx.h"
#include "http_server.h"

using namespace std;
using namespace web;
using namespace utility;
using namespace http;
using namespace web::http::experimental::listener;

http_server::http_server(utility::string_t url) : m_listener(url)
{
    m_listener.support(methods::GET, std::bind(&http_server::handle_get, this, std::placeholders::_1));
    m_listener.support(methods::PUT, std::bind(&http_server::handle_put, this, std::placeholders::_1));
    m_listener.support(methods::POST, std::bind(&http_server::handle_post, this, std::placeholders::_1));
    m_listener.support(methods::DEL, std::bind(&http_server::handle_delete, this, std::placeholders::_1));
}

void http_server::handle_get(http_request message)
{
    message.reply(status_codes::OK);
};

void http_server::handle_post(http_request message)
{
	message.reply(status_codes::Created);
};

void http_server::handle_delete(http_request message)
{
	message.reply(status_codes::OK);
}

void http_server::handle_put(http_request message)
{
	message.reply(status_codes::OK);
};

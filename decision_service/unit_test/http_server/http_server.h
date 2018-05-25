#pragma once

using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

class http_server
{
public:
	http_server(const bool post_err = false) : _post_err(post_err) {}
	http_server(const utility::string_t& url, const bool posterr = false);

	pplx::task<void> open() { return m_listener.open(); }
	pplx::task<void> close() { return m_listener.close(); }

private:

	void handle_get(http_request message);
	void handle_put(http_request message);
	void handle_post(http_request message);
	void handle_delete(http_request message);

	http_listener m_listener;
  bool _post_err;
};

class http_helper
{
	std::unique_ptr<http_server> g_http;

public:
	void on_initialize(const string_t& address, const bool post_error = false)
	{
		// Build our listener's URI from the configured address and the hard-coded path "MyServer/Action"
		uri_builder uri(address);
	  const auto addr = uri.to_uri().to_string();
		g_http = std::unique_ptr<http_server>(new http_server(addr, post_error));
		g_http->open().wait();

		return;
	}

	void on_shutdown()
	{
		g_http->close().wait();
		return;
	}
};

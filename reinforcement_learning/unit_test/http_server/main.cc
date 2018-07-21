#include "stdafx.h"
#include "http_server.h"

using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

std::unique_ptr<http_server> g_http;

void on_initialize(const string_t& address)
{
    // Build our listener's URI from the configured address and the hard-coded path "MyServer/Action"

    uri_builder uri(address);

    auto addr = uri.to_uri().to_string();
	g_http = std::unique_ptr<http_server>(new http_server(addr));
	g_http->open().wait();

    std::cout << utility::string_t(U("Listening for requests at: ")) << addr << std::endl;
    std::cout << "Press ENTER to exit." << std::endl;

    return;
}

void on_shutdown()
{
	g_http->close().wait();
    return;
}

int main()
{
    utility::string_t port = U("8080");

    utility::string_t address = U("http://localhost:");
    address.append(port);

    on_initialize(address);

    std::string line;
    std::getline(std::cin, line);

    on_shutdown();
    return 0;
}
#pragma once

#include "ds_configuration.h"

#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <openssl/hmac.h>

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;
using namespace ::pplx;
using namespace std::chrono;

//#include <exception>

namespace decision_service {

	class eventhub {
	public:

		void send(const std::string& data);
		eventhub(configuration config, const std::string& url, const std::string& name);

	private:
		pplx::task<void> post(const std::string& data);
		std::string& authorization();

	private:
		configuration _config;
		http_client _client;
		std::string _eh_name;
		long long _authorization_valid_until;// in seconds from epoch
		std::string _authorization;
	};
}

#include <openssl/hmac.h>

#include "ds_eventhub.h"

#include <sstream>

using namespace std::chrono;
using namespace utility;     // Common utilities like string conversions
using namespace web;         // Common features like URIs.
using namespace web::http;   // Common HTTP functionality

namespace decision_service {

	void eventhub::send(const std::string& data) {
		// Wait for all the outstanding I/O to complete and handle any exceptions
		try
		{
			post(data).wait();
		}
		catch (const std::exception& e)
		{
			//TODO report error
			//TODO retry
			std::cout << "Exception: " << e.what() << std::endl;
		}
	}

	pplx::task<void> eventhub::post(const std::string& data)
	{
		http_request request(methods::POST);
		request.headers().add(_XPLATSTR("Authorization"), authorization().c_str());
		request.headers().add(_XPLATSTR("Host"), _eventhub_host.c_str());

		request.set_body(data);

		return _client.request(request).then([](http_response response)
		{
			//TODO report error
		});
	}

	//private helper
	static string_t build_url(const std::string& host, const std::string& name)
	{
		//for tests
		size_t pos = host.find("localhost");
		if (pos != std::string::npos)
			return conversions::to_string_t("http://" + host);

		std::ostringstream url;
		url << "https://" << host << "/" << name << "/messages?timeout=60&api-version=2014-01";
		auto p = conversions::to_string_t(url.str());
		return p;
	}

	eventhub::eventhub(const std::string& host, const std::string& key_name, const std::string& key, const std::string& name)
		: _client(build_url(host, name)),
		_eventhub_host(host), _shared_access_key_name(key_name), _shared_access_key(key), _eventhub_name(name)
	{
		//init authorize token
		authorization();
	}

	//for tests
	eventhub::eventhub(const std::string& url)
		: _client(conversions::to_string_t(url)),
		_eventhub_host(""), _shared_access_key_name(""), _shared_access_key(""), _eventhub_name("")
	{}

	std::string& eventhub::authorization()
	{
		auto now = duration_cast<std::chrono::seconds>(system_clock::now().time_since_epoch()).count();

		// re-create authorization token if needed
		if (now > _authorization_valid_until - 60 * 15)
		{
			_authorization_valid_until = now + 60 * 60 * 24 * 7;// 1 week

			// construct "sr" 
			std::ostringstream resource_stream;
			resource_stream << "https://" << _eventhub_host << "/" << _eventhub_name;

			// encode(resource_stream)
			std::string encoded_uri = conversions::to_utf8string(web::uri::encode_data_string(conversions::to_string_t(resource_stream.str())));

			// construct data to be signed
			std::ostringstream data_stream;
			data_stream << encoded_uri << "\n" << _authorization_valid_until;
			std::string data = data_stream.str();

			// compute HMAC of data
			std::vector<unsigned char> digest(EVP_MAX_MD_SIZE);
			unsigned int digest_len;

			// https://www.openssl.org/docs/man1.0.2/crypto/hmac.html
			if (!HMAC(EVP_sha256(), _shared_access_key.c_str(), (int)_shared_access_key.length(), (const unsigned char*)data.c_str(), (int)data.length(), &digest[0], &digest_len))
			{
				// TODO: throw proper
				throw "failed to generate SAS hash";
			}

			digest.resize(digest_len);

			// encode digest (base64 + url encoding)
			auto encoded_digest = web::uri::encode_data_string(conversions::to_base64(digest));

			// construct SAS
			std::ostringstream authorization_stream;
			authorization_stream
				<< "SharedAccessSignature sr=" << encoded_uri
				<< "&sig=" << conversions::to_utf8string(encoded_digest)
				<< "&se=" << _authorization_valid_until
				<< "&skn=" << _shared_access_key_name;

			_authorization = authorization_stream.str();
		}

		return _authorization;
	}
}

#include "ds_eventhub.h"


namespace decision_service {

	void eventhub::send(const std::string& data) {
		// Wait for all the outstanding I/O to complete and handle any exceptions
		try
		{
			post(data).wait();
		}
		catch (const std::exception &e)
		{
			//TODO report error
			//TODO add retry
			std::cout << "Exception: " << e.what() << std::endl;
		}
	}

	pplx::task<void> eventhub::post(const std::string& data)
	{
		http_request request(methods::POST);
		request.headers().add(_XPLATSTR("Authorization"), authorization().c_str());
		request.headers().add(_XPLATSTR("Host"), _config.eventhub_host().c_str());
		request.set_body(data);

		return _client.request(request).then([](http_response response)
		{
			if (response.status_code() != status_codes::Created)
				std::cout << "http code = " << response.status_code() << std::endl;
		});
	}

	eventhub::eventhub(configuration config, const std::string& url, const std::string& name)
		: _config(config),
		_client(conversions::to_string_t(url)),
		_eh_name(name),
		_authorization_valid_until(0)
	{}

	std::string& eventhub::authorization()
	{
		auto now = duration_cast<std::chrono::seconds>(system_clock::now().time_since_epoch()).count();

		// re-create authorization token if needed
		if (now > _authorization_valid_until - 60 * 15)
		{
			_authorization_valid_until = now + 60 * 60 * 24 * 7 /* week */;

			// construct "sr" 
			std::ostringstream resource_stream;
			resource_stream << "https://" << _config.eventhub_host() << "/" << _eh_name;

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
			if (!HMAC(EVP_sha256(), _config.eventhub_key_value().c_str(), (int)_config.eventhub_key_value().length(), (const unsigned char*)data.c_str(), (int)data.length(), &digest[0], &digest_len))
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
				<< "&skn=" << _config.eventhub_key_name();

			_authorization = authorization_stream.str();
		}

		return _authorization;
	}

}

#include "event_hub_client.h"

#include <vector>
#include <chrono>

#include <cpprest/http_client.h>
#include <cpprest/base_uri.h>
#include <cpprest/interopstream.h>

#include <openssl/hmac.h>

#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/regex.hpp>

namespace Microsoft {
  namespace DecisionService {
    using namespace std;
    using namespace std::chrono;
    using namespace std::chrono_literals;
    using namespace utility;
    using namespace web::http;
    using namespace web::http::client;
    using namespace concurrency::streams;

    std::string decode64(const std::string &val) {
      using namespace boost::archive::iterators;
      using It = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;
      return boost::algorithm::trim_right_copy_if(std::string(It(std::begin(val)), It(std::end(val))), [](char c) {
        return c == '\0';
      });
    }

    std::string encode64(const std::string &val) {
      using namespace boost::archive::iterators;
      using It = base64_from_binary<transform_width<std::string::const_iterator, 6, 8>>;
      auto tmp = std::string(It(std::begin(val)), It(std::end(val)));
      return tmp.append((3 - val.size() % 3) % 3, '=');
    }

    // Endpoint=sb://fooo.servicebus.windows.net/;SharedAccessKeyName=RootManageSharedAccessKey;SharedAccessKey=bar;EntityPath=interaction
    boost::regex event_hub_connection_string_expression("^Endpoint=sb://([^;]+)/;SharedAccessKeyName=([^;]+);SharedAccessKey=([^;]+);EntityPath=(.+)$");

    EventHubClient::EventHubClient(std::string connection_string, bool certificate_validation_enabled)
      : _connection_string(connection_string), _authorization_valid_until(0)
    {
      // extract event hub connection string information
      boost::cmatch what;
      if (!boost::regex_match(_connection_string.c_str(), what, event_hub_connection_string_expression))
        throw "unable to parse connection string. TODO";

      _event_hub_namespace_host = string(what[1].first, what[1].second);
      _key_name = string(what[2].first, what[2].second);
      _key = string(what[3].first, what[3].second);
      _event_hub = string(what[4].first, what[4].second);

      // construct URL
      ostringstream url;
      int timeout = 60;
      url << "https://" << _event_hub_namespace_host << "/" << _event_hub << "/messages?timeout=" << timeout << "&api-version=2014-01";

      http_client_config config;

      // mainly for unit tests
      config.set_validate_certificates(certificate_validation_enabled);

      // useful when debugging with fiddler
      //config.set_proxy(web_proxy(U("http://127.0.0.1:8888")));

      _client = make_unique<http_client>(conversions::to_string_t(url.str()), config);
    }

    EventHubClient::EventHubClient(const EventHubClient& other)
      : EventHubClient(other._connection_string, other._client->client_config().validate_certificates())
    { }

    pplx::task<http_response> EventHubClient::Send(const char* data)
    {
      http_request req(methods::POST);

      req.headers().add(U("Authorization"), Authorization().c_str());
      req.headers().add(U("Host"), _event_hub_namespace_host.c_str());

      req.set_body(conversions::to_string_t(data), U("application/atom+xml;type=entry;charset=utf-8"));

      return _client->request(req);
    }

    pplx::task<http_response> EventHubClient::Send(vector<unsigned char>* data)
    {
      // create header
      http_request req(methods::POST);

      // TODO: refactor
      req.headers().add(U("Authorization"), Authorization().c_str());
      req.headers().add(U("Host"), _event_hub_namespace_host.c_str());
      req.headers().add(U("Content-Type"), "application/atom+xml;type=entry;charset=utf-8");

      req.set_body(*data);

      // TODO: handle errors
      // 1. 201 -> ok
      // 2. ??? -> if we exceed capacity, retry with dela
      // 3. 401 -> access denied
      return _client->request(req).then([=](http_response resp) { delete data; return resp; });
    }

    std::string& EventHubClient::Authorization()
    {
      auto now = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();

      // re-create authorization token if needed
      if (now > _authorization_valid_until - 60 * 15)
      {
        _authorization_valid_until = now + 60 * 60 * 24 * 7 /* week */;

        // construct "sr" 
        ostringstream resource_stream;
        resource_stream << "https://" << _event_hub_namespace_host << "/" << _event_hub;

        // encode(resource_stream)
        string encoded_uri = conversions::to_utf8string(web::uri::encode_data_string(conversions::to_string_t(resource_stream.str())));

        // to get equivalent result as C# SharedAccessSignatureTokenProvider.GetSharedAccessSignature lowercase the %xx
        // boost::regex fix_encoding_casing_expression("%[0-9A-F][0-9A-F]");
        // encoded_uri = boost::regex_replace(encoded_uri, fix_encoding_casing_expression, "\\L$0\\E");

        // construct data to be signed
        ostringstream data_stream;
        data_stream << encoded_uri << "\n" << _authorization_valid_until;
        string data = data_stream.str();

        // compute HMAC of data
        vector<unsigned char> digest(EVP_MAX_MD_SIZE);
        unsigned int digest_len;

        // https://www.openssl.org/docs/man1.0.2/crypto/hmac.html
        if (!HMAC(EVP_sha256(), _key.c_str(), (int)_key.length(), (const unsigned char*)data.c_str(), (int)data.length(), &digest[0], &digest_len))
          throw "failed to generate SAS hash"; // TODO: throw proper

        digest.resize(digest_len);
        // TODO: double check that the content is valid

        // encode digest (base64 + url encoding)
        auto encoded_digest = web::uri::encode_data_string(conversions::to_base64(digest));

        // construct SAS
        ostringstream authorization_stream;
        authorization_stream
          << "SharedAccessSignature sr=" << encoded_uri
          << "&sig=" << conversions::to_utf8string(encoded_digest)
          << "&se=" << _authorization_valid_until
          << "&skn=" << _key_name;

        _authorization = authorization_stream.str();
      }

      return _authorization;
    }
  }
}

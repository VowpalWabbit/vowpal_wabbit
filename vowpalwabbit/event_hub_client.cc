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

namespace ds {
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

  EventHubClient::EventHubClient(std::string connection_string)
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

    // useful when debugging with fiddler
    //config.set_validate_certificates(false);
    //config.set_proxy(web_proxy(U("http://127.0.0.1:8888")));

    _client = make_unique<http_client>(U(url.str()), config);
  }

  EventHubClient::EventHubClient(const EventHubClient& other)
    : EventHubClient(other._connection_string)
  { }

  pplx::task<http_response> EventHubClient::Send(const char* data)
  {
    // create header
    http_request req(methods::POST);

    req.headers().add(U("Authorization"), Authorization());
    req.headers().add(U("Host"), _event_hub_namespace_host);

    req.set_body(data, U("application/atom+xml;type=entry;charset=utf-8"));

    // TODO: handle errors
    // 1. 201 -> ok
    // 2. ??? -> if we exceed capacity, retry with dela
    // 3. 401 -> access denied
    return _client->request(req);
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

      string encoded_uri = web::uri::encode_data_string(resource_stream.str());

      // to get equivalent result as C# SharedAccessSignatureTokenProvider.GetSharedAccessSignature lowercase the %xx
      // boost::regex fix_encoding_casing_expression("%[0-9A-F][0-9A-F]");
      // encoded_uri = boost::regex_replace(encoded_uri, fix_encoding_casing_expression, "\\L$0\\E");

      // construct data to be signed
      ostringstream data_stream;
      data_stream << encoded_uri << "\n" << _authorization_valid_until;
      string data = data_stream.str();

      // compute HMAC of data
      unsigned char digest[EVP_MAX_MD_SIZE];
      unsigned int digest_len;

      // https://www.openssl.org/docs/man1.0.2/crypto/hmac.html
      if (!HMAC(EVP_sha256(), _key.c_str(), _key.length(), (const unsigned char*)data.c_str(), data.length(), digest, &digest_len))
        throw "failed to generate SAS hash"; // TODO: throw proper

      // encode digest (base64 + url encoding)
      auto encoded_digest = web::uri::encode_data_string(encode64(string((const char*)digest, digest_len)));

      // construct SAS
      ostringstream authorization_stream;
      authorization_stream
        << "SharedAccessSignature sr=" << encoded_uri
        << "&sig=" << encoded_digest
        << "&se=" << _authorization_valid_until
        << "&skn=" << _key_name;

      _authorization = authorization_stream.str();
    }

    return _authorization;
  }
}

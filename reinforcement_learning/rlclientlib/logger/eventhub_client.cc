#include <openssl/hmac.h>
#include <sstream>
#include "eventhub_client.h"
#include "err_constants.h"
#include "utility/http_helper.h"

using namespace std::chrono;
using namespace utility; // Common utilities like string conversions
using namespace web; // Common features like URIs.
using namespace web::http; // Common HTTP functionality
namespace u = reinforcement_learning::utility;

namespace reinforcement_learning {
  eventhub_client::task_context::task_context()
  {}

  eventhub_client::task_context::task_context(web::http::client::http_client& client,
    const std::string& host,
    const std::string& auth,
    std::string&& post_data)
    : _post_data(std::move(post_data))
  {
    http_request request(methods::POST);
    request.headers().add(_XPLATSTR("Authorization"), auth.c_str());
    request.headers().add(_XPLATSTR("Host"), host.c_str());
    request.set_body(_post_data.c_str());

    _task = client.request(request).then([&](http_response response) {
      return response.status_code();
    });
  }

  eventhub_client::task_context::task_context(task_context&& other)
    : _post_data(std::move(other._post_data))
    , _task(std::move(other._task))
  {}

  eventhub_client::task_context& eventhub_client::task_context::operator=(task_context&& other) {
    if (&other != this) {
      _post_data = std::move(other._post_data);
      _task = std::move(other._task);
    }
    return *this;
  }

  std::string eventhub_client::task_context::post_data() const {
    return _post_data;
  }

  web::http::status_code eventhub_client::task_context::join() {
    _task.wait();
    return _task.get();
  }

  //private helper
  string_t build_url(const std::string& host, const std::string& name, const bool local_test) {
    const std::string proto = local_test ? "http://" : "https://";
    std::string url;
    if (local_test) { url.append(proto).append(host); }
    else {
      url.append(proto).append(host).append("/").append(name)
         .append("/messages?timeout=60&api-version=2014-01");
    }
    return conversions::to_string_t(url);
  }

  int eventhub_client::init(api_status* status) { return authorization(status); }

  int eventhub_client::pop_task(api_status* status) {
    task_context oldest;
    _tasks.pop(&oldest);
    const auto status_code = oldest.join();
    if (status_code != status_codes::Created)
    {
      RETURN_ERROR_ARG(status, http_bad_status_code, "(expected 201): Found ",
        status_code, "eh_host", _eventhub_host, "eh_name", _eventhub_name,
        "\npost_data: ", oldest.post_data());
    }
    return error_code::success;
  }

  int eventhub_client::submit_task(task_context&& task, api_status* status) {
    if (_tasks.size() >= _tasks_count) {
      RETURN_IF_FAIL(pop_task(status));
    }
    _tasks.push(std::move(task));
    return error_code::success;;
  }

  int eventhub_client::v_send(std::string&& post_data, api_status* status) {
    if (authorization(status) != error_code::success)
      return status->get_error_code();
    std::string auth_str;
    {
      // protected access for _authorization
      std::lock_guard<std::mutex> lock(_mutex);
      auth_str = _authorization;
    }

    try {
      auto task = task_context(_client, _eventhub_host, auth_str, std::move(post_data));
      RETURN_IF_FAIL(submit_task(std::move(task), status));
    }
    catch (const std::exception& e) {
      RETURN_ERROR_LS(status, eventhub_http_generic) << e.what() << ", post_data: " << post_data;
    }
    return error_code::success;
  }

  eventhub_client::eventhub_client(const std::string& host, const std::string& key_name,
                                   const std::string& key, const std::string& name, size_t tasks_count, const bool local_test)
    : _client(build_url(host, name, local_test), u::get_http_config()),
      _eventhub_host(host), _shared_access_key_name(key_name),
      _shared_access_key(key), _eventhub_name(name),
      _authorization_valid_until(0), _tasks_count(tasks_count) { }

  eventhub_client::~eventhub_client() {
    while (_tasks.size() != 0) {
      pop_task(nullptr);
    }
  }

  int eventhub_client::authorization(api_status* status) {
    const auto now = duration_cast<std::chrono::seconds>(system_clock::now().time_since_epoch()).count();
    std::lock_guard<std::mutex> lock(_mutex);
    // re-create authorization token if needed
    if (now > _authorization_valid_until - 60 * 15) {
      _authorization_valid_until = now + 60 * 60 * 24 * 7; // 1 week
      // construct "sr"
      std::ostringstream resource_stream;
      resource_stream << "https://" << _eventhub_host << "/" << _eventhub_name;
      // encode(resource_stream)
      const auto encoded_uri = conversions::to_utf8string(
        web::uri::encode_data_string(conversions::to_string_t(resource_stream.str())));
      // construct data to be signed
      std::ostringstream data_stream;
      data_stream << encoded_uri << "\n" << _authorization_valid_until;
      std::string data = data_stream.str();
      // compute HMAC of data
      std::vector<unsigned char> digest(EVP_MAX_MD_SIZE);
      unsigned int digest_len;
      // https://www.openssl.org/docs/man1.0.2/crypto/hmac.html
      if (!HMAC(EVP_sha256(), _shared_access_key.c_str(), (int)_shared_access_key.length(),
                (const unsigned char*)data.c_str(), (int)data.length(), &digest[0], &digest_len)) {
        api_status::try_update(status, error_code::eventhub_generate_SAS_hash,
                               "Failed to generate SAS hash");
        return error_code::eventhub_generate_SAS_hash;
      }
      digest.resize(digest_len);
      // encode digest (base64 + url encoding)
      const auto encoded_digest = web::uri::encode_data_string(conversions::to_base64(digest));
      // construct SAS
      std::ostringstream authorization_stream;
      authorization_stream
        << "SharedAccessSignature sr=" << encoded_uri
        << "&sig=" << conversions::to_utf8string(encoded_digest)
        << "&se=" << _authorization_valid_until
        << "&skn=" << _shared_access_key_name;
      _authorization = authorization_stream.str();
    }
    return error_code::success;
  }
}

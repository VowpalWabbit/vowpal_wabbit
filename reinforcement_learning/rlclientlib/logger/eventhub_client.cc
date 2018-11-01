#include <openssl/hmac.h>
#include <sstream>
#include "eventhub_client.h"
#include "err_constants.h"
#include "utility/http_helper.h"
#include "trace_logger.h"
#include "error_callback_fn.h"
#include "str_util.h"

using namespace std::chrono;
using namespace utility; // Common utilities like string conversions
using namespace web; // Common features like URIs.
using namespace web::http; // Common HTTP functionality
namespace u = reinforcement_learning::utility;

// Private helper
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

namespace reinforcement_learning {
  eventhub_client::http_request_task::http_request_task(
    web::http::client::http_client* client,
    const std::string& host,
    const std::string& auth,
    std::string&& post_data,
    size_t max_retries,
    error_callback_fn* error_callback,
    i_trace* trace)
    : _client(client),
    _host(host),
    _auth(auth),
    _post_data(std::move(post_data)),
    _max_retries(max_retries),
    _error_callback(error_callback),
    _trace(trace)
    {
      _task = send_request(0 /* inital try */);
    }

  pplx::task<web::http::status_code> eventhub_client::http_request_task::send_request(size_t try_count) {
    http_request request(methods::POST);
    request.headers().add(_XPLATSTR("Authorization"), _auth.c_str());
    request.headers().add(_XPLATSTR("Host"), _host.c_str());
    request.set_body(_post_data.c_str());

    return _client->request(request).then([this, try_count](pplx::task<http_response> response) {
      web::http::status_code code = status_codes::InternalError;
      api_status status;

      try {
        code = response.get().status_code();
      }
      catch (const std::exception& e) {
        TRACE_ERROR(_trace, e.what());
      }

      // If the response is not the expected code then it has failed. Retry if possible otherwise report background error.
      if(code != status_codes::Created) {
        // Stop condition of recurison.
        if(try_count < _max_retries){
          TRACE_ERROR(_trace, "HTTP request failed, retrying...");

          // Yes, recursively send another request inside this one. If a subsequent request returns success we are good, otherwise the failure will propagate.
          return send_request(try_count + 1).get();
        }
        else {
          auto msg = utility::concat("(expected 201): Found ", code, ", failed after ", try_count, " retries.");
          api_status::try_update(&status, error_code::http_bad_status_code, msg.c_str());
          ERROR_CALLBACK(_error_callback, status);

          return code;
        }
      }

      // We have succeeded, return success.
      return code;
    });
  }

  int eventhub_client::http_request_task::join() {
    _task.get();

    // The task may have failed but was reported with the callback. This function's primary purpose
    // is to block if the task is not yet complete.
    return error_code::success;
  }

  std::string eventhub_client::http_request_task::post_data() const {
    return _post_data;
  }

  int eventhub_client::init(api_status* status) {
    RETURN_IF_FAIL(check_authorization_validity_generate_if_needed(status));
    return error_code::success;
  }

  int eventhub_client::pop_task(api_status* status) {
    // This function must be under a lock as there is a delay between popping from the queue and joining the task, but it should essentially be atomic.
    std::lock_guard<std::mutex> lock(_mutex_http_tasks);

    std::unique_ptr<http_request_task> oldest;
    _tasks.pop(&oldest);

    try {
      // This will block if the task is not complete yet.
      RETURN_IF_FAIL(oldest->join());
    }
    catch (...) {
      // Ignore if there is an exception surfaced as this should have been handled in the continuation.
      TRACE_WARN(_trace, "There should not be an exception raised in pop_task function.");
    }

    return error_code::success;
  }

  int eventhub_client::v_send(std::string&& post_data, api_status* status) {
    RETURN_IF_FAIL(check_authorization_validity_generate_if_needed(status));

    std::string auth_str;
    {
      // protected access for _authorization
      std::lock_guard<std::mutex> lock(_mutex);
      auth_str = _authorization;
    }

    try {
      // Before creating the task, ensure that it is allowed to be created.
      if (_tasks.size() >= _max_tasks_count) {
        RETURN_IF_FAIL(pop_task(status));
      }

      std::unique_ptr<http_request_task> request_task(new http_request_task(&_client, _eventhub_host, auth_str, std::move(post_data), _max_retries, _error_callback, _trace));
      _tasks.push(std::move(request_task));
    }
    catch (const std::exception& e) {
      RETURN_ERROR_LS(_trace, status, eventhub_http_generic) << e.what() << ", post_data: " << post_data;
    }
    return error_code::success;
  }

  eventhub_client::eventhub_client(const std::string& host, const std::string& key_name,
                                   const std::string& key, const std::string& name,
                                   size_t max_tasks_count, size_t max_retries,  i_trace* trace,
                                   error_callback_fn* error_callback, const bool local_test)
    : _client(build_url(host, name, local_test), u::get_http_config()),
      _eventhub_host(host), _shared_access_key_name(key_name),
      _shared_access_key(key), _eventhub_name(name),
      _authorization_valid_until(0), _max_tasks_count(max_tasks_count),
      _max_retries(max_retries),
      _trace(trace),
      _error_callback(error_callback)
  { }

  eventhub_client::~eventhub_client() {
    while (_tasks.size() != 0) {
      pop_task(nullptr);
    }
  }

  int eventhub_client::check_authorization_validity_generate_if_needed(api_status* status) {
    const auto now = duration_cast<std::chrono::seconds>(system_clock::now().time_since_epoch());
    std::lock_guard<std::mutex> lock(_mutex);
    // re-create authorization token if needed
    if (now.count() > _authorization_valid_until - 60 * 15) {
      RETURN_IF_FAIL(generate_authorization_string(
        now, _shared_access_key, _shared_access_key_name, _eventhub_host, _eventhub_name,
        _authorization, _authorization_valid_until, status, _trace));
    }
    return error_code::success;
  }

  int eventhub_client::generate_authorization_string(
    std::chrono::seconds now,
    const std::string& shared_access_key,
    const std::string& shared_access_key_name,
    const std::string& eventhub_host,
    const std::string& eventhub_name,
    std::string& authorization_string /* out */,
    long long& valid_until /* out */,
    api_status* status,
    i_trace* trace) {

    // Update saved valid_until value.
    valid_until = now.count() + 60 * 60 * 24 * 7; // 1 week

    // construct "sr"
    std::ostringstream resource_stream;
    resource_stream << "https://" << eventhub_host << "/" << eventhub_name;

    // encode(resource_stream)
    const auto encoded_uri = conversions::to_utf8string(
      web::uri::encode_data_string(conversions::to_string_t(resource_stream.str())));

    // construct data to be signed
    std::ostringstream data_stream;
    data_stream << encoded_uri << "\n" << valid_until;
    std::string data = data_stream.str();

    // compute HMAC of data
    std::vector<unsigned char> digest(EVP_MAX_MD_SIZE);
    unsigned int digest_len;
    // https://www.openssl.org/docs/man1.0.2/crypto/hmac.html
    if (!HMAC(EVP_sha256(), shared_access_key_name.c_str(), (int)shared_access_key.length(),
              (const unsigned char*)data.c_str(), (int)data.length(), &digest[0], &digest_len)) {
      api_status::try_update(status, error_code::eventhub_generate_SAS_hash,
                            "Failed to generate SAS hash");
      TRACE_ERROR(trace, "Failed to generate SAS hash");
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
      << "&se=" << valid_until
      << "&skn=" << shared_access_key_name;
    authorization_string = authorization_stream.str();

    return error_code::success;
  }
}

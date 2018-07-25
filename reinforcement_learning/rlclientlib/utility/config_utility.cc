#include <map>
#include <regex>
#include "config_utility.h"
#include "cpprest/json.h"
#include "config_collection.h"
#include "constants.h"
#include "err_constants.h"
#include "api_status.h"
#include "str_util.h"
#include "../model_mgmt/restapi_data_transport.h"

using namespace web;
using namespace utility::conversions; // string conversions utilities

namespace reinforcement_learning { namespace utility { namespace config {
  std::string load_config_json() {
    //TODO: Load appid configuration from Azure storage
    //TODO: error handling.  (return code or exception)
    return "";
  }

  const char* regex_code_str(int code) {
    switch(code) {
    case std::regex_constants::error_collate:
      return "The expression contained an invalid collating element name.";
    case std::regex_constants::error_ctype:
      return "The expression contained an invalid character class name.";
    case std::regex_constants::error_escape:
      return "The expression contained an invalid escaped character, or a trailing escape.";
    case std::regex_constants::error_backref:
      return "The expression contained an invalid back reference.";
    case std::regex_constants::error_brack:
      return "The expression contained mismatched brackets([and]).";
    case std::regex_constants::error_paren:
      return "The expression contained mismatched parentheses(( and )).";
    case std::regex_constants::error_brace:
      return "The expression contained mismatched braces({ and }).";
    case std::regex_constants::error_badbrace:
      return "The expression contained an invalid range between braces({ and }).";
    case std::regex_constants::error_range:
      return "The expression contained an invalid character range.";
    case std::regex_constants::error_space:
      return "There was insufficient memory to convert the expression into a finite state machine.";
    case std::regex_constants::error_badrepeat:
      return "The expression contained a repeat specifier(one of * ? +{) that was not preceded by a valid regular expression.";
    case std::regex_constants::error_complexity:
      return "The complexity of an attempted match against a regular expression exceeded a pre - set level.";
    case std::regex_constants::error_stack:
      return "There was insufficient memory to determine whether the regular expression could match the specified character sequence.";
    default:
      return "Undefined regex error";
    }
  }

  int parse_eventhub_conn_str(const std::string& conn_str, std::string& host, std::string& name, std::string& access_key_name, std::string& access_key, api_status* status) {
    try {
      const std::regex regex_eh_connstr("Endpoint=sb://([^/]+)[^;]+;SharedAccessKeyName=([^;]+);SharedAccessKey=([^;]+);EntityPath=([^;^\\s]+)");
      std::smatch match;
      if ( !std::regex_match(conn_str, match, regex_eh_connstr) && !( match.size() == 5 ) ) {
        RETURN_ERROR_LS(status, eh_connstr_parse_error) << conn_str;
      }
      host = match[1].str();
      access_key_name = match[2].str();
      access_key = match[3].str();
      name = match[4].str();
      return error_code::success;

    }
    catch ( const std::regex_error& e) {
      RETURN_ERROR_LS(status, eh_connstr_parse_error) << conn_str << ", regex_error: " << regex_code_str(e.code()) << ", details: " << e.what();
    }
  }

  int set_eventhub_config(const std::string& conn_str, const std::string& cfg_root, config_collection& cc, api_status* status) {
    std::string host;
    std::string name;
    std::string access_key_name;
    std::string access_key;
    RETURN_IF_FAIL(parse_eventhub_conn_str(conn_str, host, name, access_key_name, access_key, status));
    const auto topic = ".eventhub.";
    cc.set(concat(cfg_root, topic, "host").c_str(),     host.c_str());
    cc.set(concat(cfg_root, topic, "name").c_str(),     name.c_str());
    cc.set(concat(cfg_root, topic, "keyname").c_str(), access_key_name.c_str());
    cc.set(concat(cfg_root, topic, "key").c_str(),      access_key.c_str());
    return error_code::success;
  }

  std::string translate(const std::map<std::string, std::string>& from_to, const std::string& from) {
    const auto it = from_to.find(from);
    if ( it != from_to.end() )
      return it->second;
    return from;
  }

  int create_from_json(const std::string& config_json, config_collection& cc, api_status* status) {
    const char* json_names[] = {
      "ApplicationID",
      "ModelBlobUri",
      "InitialExplorationEpsilon"
      "ModelRefreshIntervalMs",
      "SendHighMaterMark",
      "QueueMaxSize",
      "BatchTimeoutMs"
    };

    const std::map<std::string, std::string> from_to = {
      { "ApplicationID"             , name::APP_ID },
      { "ModelBlobUri"              , name::MODEL_BLOB_URI },
      { "SendHighMaterMark"         , name::SEND_HIGH_WATER_MARK },
      { "QueueMaxSize"              , name::SEND_QUEUE_MAXSIZE },
      { "SendBatchIntervalMs"       , name::SEND_BATCH_INTERVAL },
      { "InitialExplorationEpsilon" , name::INITIAL_EPSILON },
      { "ModelRefreshIntervalMs"    , name::MODEL_REFRESH_INTERVAL_MS }
    };

    auto obj = json::value::parse(to_string_t(config_json));

    //look for specific field names in the json
    for (std::string name : json_names) {
      const auto wname = to_string_t(name); //cpp/json works with wstring
      if (obj.has_field(wname)) {
        const auto jvalue = obj.at(wname);
        const auto value = to_utf8string( jvalue.is_string() ? jvalue.as_string() : jvalue.serialize());
        name = translate(from_to, name);
        cc.set(name.c_str(), value.c_str());
      }
    }

    auto wname = to_string_t("EventHubInteractionConnectionString");
    if ( obj.has_field(to_string_t(wname)) ) {
      const auto value = to_utf8string(obj.at(wname).as_string());
      RETURN_IF_FAIL(set_eventhub_config(value, "interaction", cc, status));
    }

    wname = to_string_t("EventHubObservationConnectionString");
    if ( obj.has_field(to_string_t(wname)) ) {
      const auto value = to_utf8string(obj.at(wname).as_string());
      RETURN_IF_FAIL(set_eventhub_config(value, "observation", cc, status));
    }

    return error_code::success;
  }

}}}

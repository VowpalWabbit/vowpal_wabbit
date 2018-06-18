#include "config_utility.h"
#include "cpprest/json.h"
#include "config_collection.h"
#include <regex>
#include "../str_util.h"
#include <map>
#include "constants.h"

using namespace web;
using namespace utility::conversions; // string conversions utilities

namespace reinforcement_learning { namespace utility { namespace config {
  std::string load_config_json() {
    //TODO: Load appid configuration from Azure storage
    //TODO: error handling.  (return code or exception)
    return "";
  }

  void parse_eventhub_conn_str(const std::string& conn_str, std::string& host, std::string& name, std::string& access_key_name, std::string& access_key) {
    // "Endpoint=sb://ingest-yers.se.ws.net/;SharedAccessKeyName=oMSK;SharedAccessKey=sB1xoL4CtpA4=;EntityPath=ition",
    const std::regex regex_eh_connstr("Endpoint=sb://([^/]+)[^;]+;SharedAccessKeyName=([^;]+);SharedAccessKey=([^;]+);EntityPath=([^;^\\s]+)");
    std::smatch match;
    if(!std::regex_match(conn_str,match,regex_eh_connstr) && !match.size() == 5) {
      throw std::runtime_error(concat("Cannot parse eventhub connection string: ", conn_str));
    }
    host = match[1].str();
    access_key_name = match[2].str();
    access_key = match[3].str();
    name = match[4].str();
  }

  void set_eventhub_config(const std::string& conn_str, const std::string& cfg_root, config_collection& cc) {
    std::string host;
    std::string name;
    std::string access_key_name;
    std::string access_key;
    parse_eventhub_conn_str(conn_str, host, name, access_key_name, access_key);
    const auto topic = ".eventhub.";
    cc.set(concat(cfg_root, topic, "host").c_str(),     host.c_str());
    cc.set(concat(cfg_root, topic, "name").c_str(),     name.c_str());
    cc.set(concat(cfg_root, topic, "key_name").c_str(), access_key_name.c_str());
    cc.set(concat(cfg_root, topic, "key").c_str(),      access_key.c_str());
  }

  std::string translate(const std::map<std::string, std::string>& from_to, const std::string& from) {
    const auto it = from_to.find(from);
    if ( it != from_to.end() )
      return it->second;
    return from;
  }

  config_collection create_from_json(const std::string& config_json) {
    config_collection cc;
    const char* json_names[] = {
      "ApplicationID",
      "ModelBlobUri",
      "InitialExplorationEpsilon",
      "ModelRefreshPeriodMs",
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
      { "ModelRefreshIntervalMs"    , name::MODEL_REFRESH_INTERVAL }
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
      set_eventhub_config(value, "interaction", cc);
    }

    wname = to_string_t("EventHubObservationConnectionString");
    if ( obj.has_field(to_string_t(wname)) ) {
      const auto value = to_utf8string(obj.at(wname).as_string());
      set_eventhub_config(value, "observation", cc);
    }

    return cc;
  }

}}}

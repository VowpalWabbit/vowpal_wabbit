#include "rl.net.native.h"

#pragma once

//int load_file(const std::string& file_name, std::string& file_data);
//int load_config_from_json(const std::string& file_name, u::configuration& cc);

// Exports
extern "C" {
    // NOTE: THIS IS NOT POLYMORPHISM SAFE!
    API u::configuration* CreateConfig();
    API void DeleteConfig(u::configuration* config);

    API int LoadConfigFromJson(const int json_length, const char* json_value, u::configuration);
    //Add
    //Remove?
    //LoadJson
}
#include "rl.net.native.h"

#pragma once

// Exports
extern "C" {
    // NOTE: THIS IS NOT POLYMORPHISM SAFE!
    API reinforcement_learning::utility::configuration* CreateConfig();
    API void DeleteConfig(reinforcement_learning::utility::configuration* config);

    API int LoadConfigurationFromJson(const int json_length, const char* json_value, reinforcement_learning::utility::configuration* config, reinforcement_learning::api_status* status = nullptr);
    //Add
    //Remove?
}
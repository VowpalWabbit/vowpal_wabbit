#pragma once

#include <iostream>
#include <fstream>
#include "config_utility.h"
#include "live_model.h"

// Namespace manipulation for brevity
namespace r = reinforcement_learning;
namespace u = r::utility;
namespace cfg = u::config;
namespace err = r::error_code;

#define API __declspec(dllexport)
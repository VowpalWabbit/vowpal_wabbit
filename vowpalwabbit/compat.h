#pragma once

#include "learner.h"
#include "options.h"
#include "global_data.h"
#include "io_buf.h"
#include "future_compat.h"

#include <string>

// Deprecated. Use VW::LEARNER directly.
namespace LEARNER = VW::LEARNER;

namespace VW
{
  VW_DEPRECATED("Use vw::initialize")
  inline vw* initialize(VW::config::options_i& options, io_buf* model = nullptr, bool skipModelLoad = false,
      trace_message_t trace_listener = nullptr, void* trace_context = nullptr)
  {
    auto vw_obj = vw::initialize(options, model, skipModelLoad, trace_listener, trace_context);
    vw_obj->is_managed_by_unique = false;
    return vw_obj.release();
  }

  VW_DEPRECATED("Use vw::initialize")
  inline vw* initialize(std::string s, io_buf* model = nullptr, bool skipModelLoad = false,
      trace_message_t trace_listener = nullptr, void* trace_context = nullptr)
  {
    auto vw_obj = vw::initialize(s, model, skipModelLoad, trace_listener, trace_context);
    vw_obj->is_managed_by_unique = false;
    return vw_obj.release();
  }

  VW_DEPRECATED("Use vw::initialize")
  inline vw* initialize(int argc, char* argv[], io_buf* model = nullptr, bool skipModelLoad = false,
      trace_message_t trace_listener = nullptr, void* trace_context = nullptr)
  {
    auto vw_obj = vw::initialize(argc, argv, model, skipModelLoad, trace_listener, trace_context);
    vw_obj->is_managed_by_unique = false;
    return vw_obj.release();
  }

  VW_DEPRECATED("Use vw::seed_vw_model")
  inline vw* seed_vw_model(
      vw* vw_model, std::string extra_args, trace_message_t trace_listener = nullptr, void* trace_context = nullptr)
  {
    auto vw_obj = vw::seed_vw_model(vw_model, extra_args, trace_listener, trace_context);
    vw_obj->is_managed_by_unique = false;
    return vw_obj.release();
  }

  // Allows the input command line string to have spaces escaped by '\'
  VW_DEPRECATED("Use vw::initialize_escaped")
  inline vw* initialize_escaped(std::string const& s, io_buf* model = nullptr, bool skipModelLoad = false,
      trace_message_t trace_listener = nullptr, void* trace_context = nullptr)
  {
    auto vw_obj = vw::initialize_escaped(s, model, skipModelLoad, trace_listener, trace_context);
    vw_obj->is_managed_by_unique = false;
    return vw_obj.release();
  }
}
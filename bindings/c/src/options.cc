// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/experimental/options.h"

VW_DLL_PUBLIC VWStatus vw_create_options(VWOptions** options_handle, VWErrorInfo* err_info_container) noexcept
{
  return VW_not_implemented;
}

VW_DLL_PUBLIC VWStatus vw_create_options_from_command_line(
    int argc, char** argv, VWOptions** options_handle, VWErrorInfo* err_info_container) noexcept
{
  return VW_not_implemented;
}

VW_DLL_PUBLIC VWStatus vw_create_options_from_command_line_cstring(
    const char* command_line, VWOptions** options_handle, VWErrorInfo* err_info_container) noexcept
{
  return VW_not_implemented;
}

VW_DLL_PUBLIC VWStatus vw_destroy_options(VWOptions* options_handle, VWErrorInfo* err_info_container) noexcept
{
  return VW_not_implemented;
}

VW_DLL_PUBLIC VWStatus vw_options_set_string(VWOptions* options_handle, const char* option_name,
    const char* option_value, VWErrorInfo* err_info_container) noexcept
{
  return VW_not_implemented;
}

VW_DLL_PUBLIC VWStatus vw_options_set_integer(
    VWOptions* options_handle, const char* option_name, int32_t option_value, VWErrorInfo* err_info_container) noexcept
{
  return VW_not_implemented;
}

VW_DLL_PUBLIC VWStatus vw_options_set_float(
    VWOptions* options_handle, const char* option_name, float option_value, VWErrorInfo* err_info_container) noexcept
{
  return VW_not_implemented;
}

VW_DLL_PUBLIC VWStatus vw_options_set_bool(
    VWOptions* options_handle, const char* option_name, bool option_value, VWErrorInfo* err_info_container) noexcept
{
  return VW_not_implemented;
}

VW_DLL_PUBLIC VWStatus vw_options_list_push_string(VWOptions* options_handle, const char* option_name,
    const char* option_value, VWErrorInfo* err_info_container) noexcept
{
  return VW_not_implemented;
}

VW_DLL_PUBLIC VWStatus vw_options_list_push_integer(
    VWOptions* options_handle, const char* option_name, int32_t option_value, VWErrorInfo* err_info_container) noexcept
{
  return VW_not_implemented;
}

VW_DLL_PUBLIC VWStatus vw_options_list_push_float(
    VWOptions* options_handle, const char* option_name, float option_value, VWErrorInfo* err_info_container) noexcept
{
  return VW_not_implemented;
}
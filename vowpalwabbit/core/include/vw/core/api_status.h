// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <string>

// TODO This file contains an experimental defintion of api_status - the design
// of this will be updated once
// https://github.com/VowpalWabbit/vowpal_wabbit/pull/2493 is merged.

namespace VW
{
namespace experimental
{
/**
 * @brief Report status of all API calls
 */
class api_status
{
public:
  api_status();

  /**
   * @brief (\ref api_error_codes) Get the error code
   * All API calls will return a status code.  If the optional api_status object is
   * passed in, the code is set in the object also.
   * @return int Error code
   */
  int get_error_code() const;

  /**
   * @brief (\ref api_error_codes) Get the error msg string
   * All API calls will return a status code.  If the optional api_status object is
   * passed in, the detailed error description is passed back using get_error_msg()
   * @return const char* Error description
   */
  const char* get_error_msg() const;

  /**
   * @brief Helper method for returning error object
   * Checks to see if api_status is not null before setting the error code and error message
   * @param status Error object.  Could be null.
   * @param new_code Error code to set if status object is not null
   * @param new_msg Error description to set if status object is not null
   */
  static void try_update(api_status* status, int new_code, const char* new_msg);

  /**
   * @brief Helper method to clear the error object
   * Checks to see if api_status is not null before clearing current values.
   * @param status Error object.  Could be null.
   */
  static void try_clear(api_status* status);

private:
  int _error_code;
  std::string _error_msg;
};
}  // namespace experimental
}  // namespace VW

/**
 * @brief Error reporting macro that takes a list of parameters
 */
#ifndef RETURN_ERROR
#  define RETURN_ERROR(status, code)                                                           \
    do                                                                                         \
    {                                                                                          \
      VW::experimental::api_status::try_update(                                                \
          status, VW::experimental::error_code::code, VW::experimental::error_code::code##_s); \
      return VW::experimental::error_code::code;                                               \
    } while (0);
#endif

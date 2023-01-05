// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <sstream>
#include <string>

// TODO This file contains an experimental defintion of api_status - the design
// of this will be updated once
// https://github.com/VowpalWabbit/vowpal_wabbit/pull/2493 is merged.

namespace VW
{
namespace experimental
{
using i_trace = void;

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

/**
 * @brief Helper class used in report_error template funcstions to return status from API calls.
 */
class status_builder
{
public:
  /**
   * @brief Construct a new status builder object
   *
   * @param trace i_trace object which can be null
   * @param status api_status object which can be null
   * @param code Error code
   */
  status_builder(i_trace* trace, api_status* status, int code);
  ~status_builder();

  //! return the status when cast to an int
  operator int() const;

  //! Error code
  int _code;
  //! Api status object which can be null
  api_status* _status;
  //! String stream used to serialize detailed error message
  std::ostringstream _os;

  status_builder(const status_builder&&) = delete;
  status_builder(const status_builder&) = delete;
  status_builder& operator=(const status_builder&) = delete;
  status_builder& operator=(status_builder&&) = delete;

private:
  //! Trace logger
  i_trace* _trace;
  //! Is logging needed
  bool enable_logging() const;
};

/**
 * @brief Ends recursion of report_error variadic template with the Last argument
 *
 * @tparam Last type of Last argument to report_error
 * @param os ostringstream that contains serialized error data
 * @param last value of last argument to report_error
 */
template <typename Last>
void report_error(std::ostringstream& os, const Last& last)
{
  os << last;
}

/**
 * @brief Begins recursion of report_error variadic template with ostringstream to serialize error information
 *
 * @tparam First Type of first parameter to report_error
 * @tparam Rest Rest of the parameter type list
 * @param os ostringstream that contains serialized error data
 * @param first value of the first parameter to report_error
 * @param rest rest of the values to report error
 */
template <typename First, typename... Rest>
void report_error(std::ostringstream& os, const First& first, const Rest&... rest)
{
  os << first;
  report_error(os, rest...);
}

/**
 * @brief The main report_error template used to serialize error into api_status
 * The method most end users will use with a status code and a number of objects to be serialised into an api_status
 * @tparam All All the type arguments to the variadic template
 * @param status Status object to serialize error description into
 * @param scode Error code
 * @param all Parameter list argument for the variadic template
 * @return int Error code that was passed in
 */
template <typename... All>
int report_error(api_status* status, int scode, const All&... all)
{
  if (status != nullptr)
  {
    std::ostringstream os;
    report_error(os, all...);
    api_status::try_update(status, scode, os.str().c_str());
  }
  return scode;
}
/**
 * @brief left shift operator to serialize types into stringstream held in status_builder
 *
 * @tparam T Type to serialize
 * @param sb Status builder that holds serialized error message
 * @param val Error code
 * @return reinforcement_learning::status_builder& Passed in status builder so left shift operators can be chained
 * together.
 */
template <typename T>
VW::experimental::status_builder& operator<<(VW::experimental::status_builder& sb, const T& val)
{
  if (sb._status != nullptr) { sb._os << ", " << val; }
  return sb;
}
}  // namespace experimental
}  // namespace VW

// This is weird, but we want these to be able to use the left-shift operator, but that is undesirable to
// have in the experimental namespace because we want to avoid forcing consumers to import it. So temporarily
// pop out of the namespace to define the leftshift operator, then pop back into it.
namespace VW
{
namespace experimental
{
/**
 * @brief Terminates recursion of report_error
 *
 * @param sb status_builder that contains the serialized error string
 * @return int Error status
 */
inline int report_error(status_builder& sb) { return sb; }

/**
 * @brief report_error that takes the final paramter
 *
 * @tparam Last Final paramter type
 * @param sb status_builder that contains the serialized error string
 * @param last Final parameter value
 * @return int Error status
 */
template <typename Last>
int report_error(status_builder& sb, const Last& last)
{
  return sb << last;
}

/**
 * @brief variadic template report_error that takes a list of parameters
 *
 * @tparam First Type of first parameter in parameter list
 * @tparam Rest Tail parameter types in paramter list
 * @param sb status_builder that contains the serialized error string
 * @param first First parameter value
 * @param rest Tail paramter value list
 * @return int Error status
 */
template <typename First, typename... Rest>
int report_error(status_builder& sb, const First& first, const Rest&... rest)
{
  sb << first;
  return report_error(sb, rest...);
}
}  // namespace experimental
}  // namespace VW

#ifndef RETURN_ERROR
/**
 * @brief Error reporting macro for just returning an error code.
 */
#  define RETURN_ERROR(status, code, ...)                                                         \
    do {                                                                                          \
      if (status != nullptr)                                                                      \
      {                                                                                           \
        VW::experimental::status_builder sb(nullptr, status, VW::experimental::error_code::code); \
        sb << VW::experimental::error_code::code##_s;                                             \
        return report_error(sb);                                                                  \
      }                                                                                           \
      return VW::experimental::error_code::code;                                                  \
    } while (0);

#endif  // RETURN_ERROR

#ifndef RETURN_ERROR_ARG
/**
 * @brief Error reporting macro that takes a list of parameters
 */
#  define RETURN_ERROR_ARG(status, code, ...)                                                     \
    do {                                                                                          \
      if (status != nullptr)                                                                      \
      {                                                                                           \
        VW::experimental::status_builder sb(nullptr, status, VW::experimental::error_code::code); \
        sb << VW::experimental::error_code::code##_s;                                             \
        return report_error(sb, __VA_ARGS__);                                                     \
      }                                                                                           \
      return VW::experimental::error_code::code;                                                  \
    } while (0);

#endif  // RETURN_ERROR_ARG

#ifndef RETURN_ERROR_LS
/**
 * @brief Error reporting macro used with left shift operator
 */
#  define RETURN_ERROR_LS(status, code)                                                       \
    VW::experimental::status_builder sb(nullptr, status, VW::experimental::error_code::code); \
    return sb << VW::experimental::error_code::code##_s

#endif  // RETURN_ERROR_LS

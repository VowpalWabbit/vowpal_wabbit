/**
 * @brief api_status definition.  api_status is used to return error information to the caller. (\ref error_codes)
 *
 * @file api_status.h
 * @author Rajan Chari et al
 * @date 2018-07-18
 */
#pragma once

#include <string>
#include <sstream>
#include "err_constants.h"

namespace reinforcement_learning {
  class i_trace;
  /**
   * @brief Report status of all API calls
   */
  class api_status {
    public:
      api_status();

      /**
       * @brief (\ref error_codes) Get the error code
       * All API calls will return a status code.  If the optional api_status object is
       * passed in, the code is set in the object also.
       * @return int Error code
       */
      int get_error_code() const;

      /**
       * @brief (\ref error_codes) Get the error msg string
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
      int _error_code;          //! Error code
      std::string _error_msg;   //! Error description
  };

  /**
   * @brief Helper class used in report_error template funcstions to return status from API calls.
   */
  struct status_builder {
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
  void report_error(std::ostringstream& os, const Last& last) {
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
  template <typename First, typename ... Rest>
  void report_error(std::ostringstream& os, const First& first, const Rest& ... rest) {
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
  template <typename ... All>
  int report_error(api_status* status, int scode, const All& ... all) {
    if ( status != nullptr ) {
      std::ostringstream os;
      report_error(os, all...);
      api_status::try_update(status, scode, os.str().c_str());
    }
    return scode;
  }
}

/**
 * @brief left shift operator to serialize types into stringstream held in status_builder
 *
 * @tparam T Type to serialize
 * @param sb Status builder that holds serialized error message
 * @param val Error code
 * @return reinforcement_learning::status_builder& Passed in status builder so left shift operators can be chained together.
 */
template <typename T>
reinforcement_learning::status_builder& operator <<(reinforcement_learning::status_builder& sb, const T& val) {
  if ( sb._status != nullptr ) {
    sb._os << ", " << val;
  }
  return sb;
}

namespace reinforcement_learning {
  /**
   * @brief Terminates recursion of report_error
   *
   * @param sb status_builder that contains the serialized error string
   * @return int Error status
   */
  inline int report_error(status_builder& sb) {
    return sb;
  }

  /**
   * @brief report_error that takes the final paramter
   *
   * @tparam Last Final paramter type
   * @param sb status_builder that contains the serialized error string
   * @param last Final parameter value
   * @return int Error status
   */
  template <typename Last>
  int report_error(status_builder& sb, const Last& last) {
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
  template <typename First, typename ... Rest>
  int report_error(status_builder& sb, const First& first, const Rest& ... rest) {
    sb << first;
    return report_error(sb, rest...);
  }
}

/**
 * @brief Error reporting macro that takes a list of parameters
 */
#define RETURN_ERROR_ARG(trace, status, code, ... ) do {                                                  \
  if(status != nullptr) {                                                                                 \
    reinforcement_learning::status_builder sb(trace, status, reinforcement_learning::error_code::code);   \
    sb << reinforcement_learning::error_code::code ## _s;                                         \
    return report_error(sb, __VA_ARGS__ );                                                        \
  }                                                                                               \
  return reinforcement_learning::error_code::code;                                                \
} while(0);                                                                                       \

/**
 * @brief Error reporting macro used with left shift operator
 */
#define RETURN_ERROR_LS(trace, status, code)                                                          \
reinforcement_learning::status_builder sb(trace, status, reinforcement_learning::error_code::code);   \
return sb << reinforcement_learning::error_code::code ## _s                                           \

 /**
 * @brief Error reporting macro to test and return on error
 */
#define RETURN_IF_FAIL(x) do {    \
  int retval__LINE__ = (x);       \
  if (retval__LINE__ != 0) {      \
    return retval__LINE__;        \
  }                               \
} while (0)                       \


#pragma once
#include <string>
namespace VW
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
    int _error_code;         //! Error code
    std::string _error_msg;  //! Error description
  };

  }

/**
 * @brief Error reporting macro that takes a list of parameters
 */
#ifndef RETURN_ERROR
#define RETURN_ERROR(status, code)                                                                    \
  do                                                                                                      \
  {                                                                                                       \
    VW::api_status::try_update(status, VW::error_code::code, VW::error_code::code##_s);   \
    return VW::error_code::code;                                                      \
  } while (0);
#endif

/**
 * @brief Definition of all API error return codes and descriptions
 *
 * @file err_constants.h
 * @author Rajan Chari et al
 * @date 2018-07-18
 */
#pragma once

//! [Error Generator]
#define ERROR_CODE_DEFINITION(code, name, message) \
namespace VW { namespace error_code {\
  const int name = code;\
  char const * const name ## _s = message;\
}}
//! [Error Generator]

#include "errors_data.h"

namespace VW { namespace error_code {
  // Success code
  const int success = 0;
}}

namespace VW { namespace error_code {
  char const * const unknown_s                   = "Unexpected error.";
}}

#undef ERROR_CODE_DEFINITION

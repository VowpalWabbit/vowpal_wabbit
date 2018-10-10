/**
* @brief Definition of all API error return codes and descriptions
*
* @file err_constants.h
* @author Rajan Chari, Dwaipayan Mukhopadhyay et al
* @date 2018-07-18
*/
#pragma once

namespace vw_lib {
  namespace error_code {
    //! [Error Codes]
    const int success = 0;
    //! [Error Codes]
  }
}

namespace vw_lib {
  namespace error_code {
    //! [Error Description]
    char const * const unknown_s = "Unexpected error.";
    //! [Error Description]
  }
}

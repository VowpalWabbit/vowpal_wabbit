#pragma once
namespace r = reinforcement_learning;

//#define RETURN_ON_ERROR(scode, status)
#define RETURN_ON_ERROR(scode, status) do {                     \
  if( scode != r::error_code::success ) {                       \
    std::cout << status.get_error_msg() << std::endl;           \
    return scode;                                               \
  }                                                             \
} while ( 0 );                                                  \

//#define RETURN_ON_ERROR_STR(scode, str)
#define RETURN_ON_ERROR_STR(scode, str) do {  \
  if( scode != r::error_code::success ) {     \
    std::cout << str << std::endl;            \
    return;                                   \
  }                                           \
} while ( 0 );                                

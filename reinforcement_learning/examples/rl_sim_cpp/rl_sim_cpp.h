#pragma once
namespace r = reinforcement_learning;

#define RETURN_ON_ERROR(scodeexpr, status) do {                 \
  const auto __FILE__##scode = (scodeexpr);                     \
  if( __FILE__##scode != r::error_code::success ) {             \
    std::cout << status.get_error_msg() << std::endl;           \
    return __FILE__##scode;                                     \
  }                                                             \
} while ( 0 );                                                  \

#define RETURN_ON_ERROR_STR(scodeexpr, str) do {                \
  const auto __FILE__##scode = (scodeexpr);                     \
  if( __FILE__##scode != 0 ) {                                  \
    std::cout << str << std::endl;                              \
    return __FILE__##scode;                                     \
  }                                                             \
} while ( 0 );                                                  \


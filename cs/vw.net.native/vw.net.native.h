#pragma once

#include "vw/core/api_status.h"
#include "vw/core/error_constants.h"
#include "vw/core/v_array.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <vector>

#if defined(_MSC_VER)
//  Microsoft
#  define API __declspec(dllexport)
#elif defined(__GNUC__)
//  GCC
#  define API __attribute__((visibility("default")))
#else
//  do nothing and hope for the best?
#  define API
#  pragma warning Unknown dynamic link import / export semantics.
#endif

namespace vw_net_native
{
using dotnet_size_t = int32_t;
using dotnet_bool_u1_t = unsigned char;

using ERROR_CODE = int;

inline bool FloatEqual(float a, float b)
{
  if ((std::abs(a) < 1e-20 && std::abs(b) < 1e-20) || (std::isinf(a) && std::isinf(b))) { return true; }

  return std::abs(a - b) / std::max(a, b) < 1e-6;
}

inline char* stdstr_to_cstr(const std::string& str) { return strdup(str.c_str()); }

inline char* stringstream_to_cstr(const std::stringstream& sstream) { return stdstr_to_cstr(sstream.str()); }

inline vw_net_native::dotnet_size_t size_to_neg_dotnet_size(size_t size)
{
  return -static_cast<vw_net_native::dotnet_size_t>(size);
}

template <typename T, typename Enable = void>
struct v_iterator_context;

template <typename T>
struct v_iterator_context<T, typename std::enable_if<std::is_trivially_copyable<T>::value>::type>
{
  const v_array<T>* v;
  typename v_array<T>::const_iterator it;
};

template <typename T>
inline vw_net_native::dotnet_size_t stdvector_copy_to_managed(
    const std::vector<T>& source, T* destination, vw_net_native::dotnet_size_t count)
{
  if (count < source.size())
  {
    return size_to_neg_dotnet_size(source.size());  // Not enough space in destination buffer
  }

  std::copy(source.begin(), source.end(), destination);

  // This downcast is safe, despite being signed-to-unsigned, because we implicitly checked for
  // this overflow above, when comparing against the size of the output array.
  return (vw_net_native::dotnet_size_t)source.size();
}

template <typename T>
inline vw_net_native::dotnet_size_t stdvector_copy_n_to_managed(
    const std::vector<T>& source, T* destination, vw_net_native::dotnet_size_t limit)
{
  size_t copied_count = std::min(source.size(), static_cast<size_t>(limit));
  std::copy_n(source.begin(), copied_count, destination);

  // This downcast is safe, despite being signed-to-unsigned, because we implicitly checked for
  // this overflow above, when comparing against the size of the output array.
  return (vw_net_native::dotnet_size_t)copied_count;
}

}  // namespace vw_net_native

extern "C"
{
  API void FreeDupString(char* str);
  // API const char* LookupMessageForErrorCode(int code);

  API uint64_t VwUniformHash(char* key, size_t len, uint64_t seed);

  API size_t StdStringGetLength(const std::string* str);
  API vw_net_native::dotnet_size_t StdStringCopyToBuffer(
      const std::string* str, char* buffer, vw_net_native::dotnet_size_t count);
}

#define FILL_ERROR_LS(status, code)                                                         \
  VW::experimental::status_builder sb(nullptr, status, VW::experimental::error_code::code); \
  sb << VW::experimental::error_code::code##_s

#define CATCH_FILL_STATUS                                                                                       \
  catch (VW::vw_exception const& ex)                                                                            \
  {                                                                                                             \
    FILL_ERROR_LS(status, vw_exception) << ex.what() << "(" << ex.filename() << ":" << ex.line_number() << ")"; \
  }                                                                                                             \
  catch (std::exception const& ex) { FILL_ERROR_LS(status, native_exception) << ex.what(); }

#define CATCH_RETURN_STATUS                                                                                       \
  catch (VW::vw_exception const& ex)                                                                              \
  {                                                                                                               \
    RETURN_ERROR_LS(status, vw_exception) << ex.what() << "(" << ex.filename() << ":" << ex.line_number() << ")"; \
  }                                                                                                               \
  catch (std::exception const& ex) { RETURN_ERROR_LS(status, native_exception) << ex.what(); }

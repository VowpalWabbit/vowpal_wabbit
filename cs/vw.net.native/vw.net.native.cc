#include "vw.net.native.h"

#include "vw/common/hash.h"

#include <cstdlib>

API void FreeDupString(char* str) { free(str); }

API uint64_t VwUniformHash(char* key, size_t len, uint64_t seed) { return VW::uniform_hash(key, len, seed); }

API size_t StdStringGetLength(const std::string* str) { return str->size(); }

API vw_net_native::dotnet_size_t StdStringCopyToBuffer(
    const std::string* str, char* buffer, vw_net_native::dotnet_size_t count)
{
  size_t actual_count = str->copy(buffer, count);

  return static_cast<vw_net_native::dotnet_size_t>(actual_count);
}

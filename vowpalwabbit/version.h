#include <inttypes.h>
#include <cstdio>
#include <string>

#include "config.h"

namespace VW {
  struct version_struct
  {
    int32_t major;
    int32_t minor;
    int32_t rev;
    version_struct(int maj = 0, int min = 0, int rv = 0)
    {
      major = maj;
      minor = min;
      rev = rv;
    }
    version_struct(const char* v_str) { from_string(v_str); }
    void operator=(version_struct v)
    {
      major = v.major;
      minor = v.minor;
      rev = v.rev;
    }
    version_struct(const version_struct& v)
    {
      major = v.major;
      minor = v.minor;
      rev = v.rev;
    }
    void operator=(const char* v_str) { from_string(v_str); }
    bool operator==(version_struct v) { return (major == v.major && minor == v.minor && rev == v.rev); }
    bool operator==(const char* v_str)
    {
      version_struct v_tmp(v_str);
      return (*this == v_tmp);
    }
    bool operator!=(version_struct v) { return !(*this == v); }
    bool operator!=(const char* v_str)
    {
      version_struct v_tmp(v_str);
      return (*this != v_tmp);
    }
    bool operator>=(version_struct v)
    {
      if (major < v.major)
        return false;
      if (major > v.major)
        return true;
      if (minor < v.minor)
        return false;
      if (minor > v.minor)
        return true;
      if (rev >= v.rev)
        return true;
      return false;
    }
    bool operator>=(const char* v_str)
    {
      version_struct v_tmp(v_str);
      return (*this >= v_tmp);
    }
    bool operator>(version_struct v)
    {
      if (major < v.major)
        return false;
      if (major > v.major)
        return true;
      if (minor < v.minor)
        return false;
      if (minor > v.minor)
        return true;
      if (rev > v.rev)
        return true;
      return false;
    }
    bool operator>(const char* v_str)
    {
      version_struct v_tmp(v_str);
      return (*this > v_tmp);
    }
    bool operator<=(version_struct v) { return !(*this > v); }
    bool operator<=(const char* v_str)
    {
      version_struct v_tmp(v_str);
      return (*this <= v_tmp);
    }
    bool operator<(version_struct v) { return !(*this >= v); }
    bool operator<(const char* v_str)
    {
      version_struct v_tmp(v_str);
      return (*this < v_tmp);
    }
    std::string to_string() const
    {
      char v_str[128];
      sprintf_s(v_str, sizeof(v_str), "%d.%d.%d", major, minor, rev);
      std::string s = v_str;
      return s;
    }
    void from_string(const char* str)
    {
  #ifdef _WIN32
      sscanf_s(str, "%d.%d.%d", &major, &minor, &rev);
  #else
      std::sscanf(str, "%d.%d.%d", &major, &minor, &rev);
  #endif
    }
  };

  const version_struct version(PACKAGE_VERSION);
}




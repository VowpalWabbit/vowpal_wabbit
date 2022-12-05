// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#define VW_EXPAND(x) x
#define VW_GET_MACRO(_1, _2, NAME, ...) NAME
// UNUSED is used to silence the warning: warning: ISO C++11 requires at least one argument for the "..." in a variadic
// macro
#define THROW_OR_RETURN(...) \
  VW_EXPAND(VW_GET_MACRO(__VA_ARGS__, THROW_OR_RETURN_NORMAL, THROW_OR_RETURN_VOID, UNUSED)(__VA_ARGS__))

#ifdef VW_NOEXCEPT

#  define THROW_OR_RETURN_NORMAL(args, retval) \
    do {                                       \
      return retval;                           \
    } while (0)

#  define THROW_OR_RETURN_VOID(args) \
    do {                             \
      return;                        \
    } while (0)

#else  // VW_NOEXCEPT defined

#  include "vw/common/vw_exception.h"

#  include <sstream>

// ease error handling and also log filename and line number
#  define THROW(args)                                            \
    {                                                            \
      std::ostringstream __msg;                                  \
      __msg << args;                                             \
      throw VW::vw_exception(VW_FILENAME, VW_LINE, __msg.str()); \
    }

#  define THROW_EX(ex, args)                       \
    {                                              \
      std::ostringstream __msg;                    \
      __msg << args;                               \
      throw ex(VW_FILENAME, VW_LINE, __msg.str()); \
    }

#  define VW_ASSERT(condition, args) \
    if (!(condition)) { THROW(args); }

#  define THROW_OR_RETURN_NORMAL(args, retval)                    \
    do {                                                          \
      std::ostringstream __msgA;                                  \
      __msgA << args;                                             \
      throw VW::vw_exception(VW_FILENAME, VW_LINE, __msgA.str()); \
    } while (0)

#  define THROW_OR_RETURN_VOID(args)                              \
    do {                                                          \
      std::ostringstream __msgB;                                  \
      __msgB << args;                                             \
      throw VW::vw_exception(VW_FILENAME, VW_LINE, __msgB.str()); \
    } while (0)

#endif

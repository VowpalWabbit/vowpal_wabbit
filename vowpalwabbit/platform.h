/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
*/

//
// Please try to keep all platform differences in this file
// Avoid cluttering sources with #ifdef's
// Instead just do  #include "platform.h"
//
// ariel faigon - 2013-02-26
//
#ifdef _WIN32
#   include <WinSock2.h>
#   include <WS2tcpip.h>
#   include <Windows.h>
    // io_buf.h
#   include <sys/types.h>
#   include <unistd.h>
#   include <io.h>
#   include <sys/stat.h>

    typedef unsigned int uint32_t;
    typedef pthread_t HANDLE;    // global_data.h

    // parse_primitives.h
    typedef CRITICAL_SECTION    MUTEX;
    typedef CONDITION_VARIABLE  CV;

    // v_array.h
#   define  __INLINE  /*nothing*/
    // io_buf.h
#   define   ssize_t    size_t
 
    // Someone on stackoverflow suggests #define _CRT_NONSTDC_NO_DEPRECATE
    // To avoid having to use all the prefix-underscore names

    // comp_io.h, io_buf.h
#   define   fileno(fp)                 _fileno(fp)
#   define   lseek(fd, offset, how)     _lseek(fd, offset, how)
#   define   read(fd, buf, n)           _read(fd, buf, n)
#   define   write(fd, buf, n)          _write(fd, buf, n)
#   define   close(fd)                  _close(fd)
    //
    // Don't use _sopen_s, see:
    // http://stackoverflow.com/questions/6963659/what-is-the-sopen-s-equivalent-for-the-open-function
    //

    // One may chose to ignore warnings by quieting them, however
    // John feels this is not prudent, so I commented these out
    // #   define  _CRT_NONSTDC_NO_DEPRECATE
    // #   define  _SCL_SECURE_NO_WARNINGS
    // #   define  _STL_SECURE_NO_WARNINGS

    // v_array.h: so we can redefine them
#   undef max
#   undef min

#else /* not Windows */

#   include <sys/socket.h>
#   include <sys/types.h>
#   include <netinet/in.h>
#   include <netinet/tcp.h>
#   include <netdb.h>
#   include <unistd.h>

    // parse_primitives.h
    typedef pthread_mutex_t     MUTEX;
    typedef pthread_cond_t      CV;

    // v_array.h
#   define __INLINE             inline

#endif


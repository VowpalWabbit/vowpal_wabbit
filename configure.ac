AC_INIT([vowpal_wabbit], [7.10.1], [jl@hunch.net])
AC_DEFINE([PACKAGE_URL],["https://github.com/JohnLangford/vowpal_wabbit"],[project url])
AC_CONFIG_HEADERS(vowpalwabbit/config.h)
AM_INIT_AUTOMAKE()

AC_CONFIG_MACRO_DIR([acinclude.d])

# if CXXFLAGS is not given by user, don't use default of "-O2 -g"
: ${CXXFLAGS=""}

LT_INIT
AC_PROG_CXX
AC_PROG_CXXCPP
AC_LANG([C++])
AC_CANONICAL_HOST
AC_PROG_INSTALL
AX_CXX_COMPILE_STDCXX_11(noext,optional)
AS_IF([test "${HAVE_CXX11}" != 1],
  [
    AX_CXX_COMPILE_STDCXX_0X(noext,mandatory)
  ])
AC_HEADER_STDC

AX_BOOST_BASE([1.0], [], [
  AC_MSG_ERROR([Could not find a valid version of boost.])
])
AX_BOOST_PROGRAM_OPTIONS

AX_CHECK_ZLIB
AS_IF([test "x${ZLIB_HOME}" != x],
  [
    ZLIB_CPPFLAGS="-I${ZLIB_HOME}/include"
    ZLIB_LDFLAGS="-L${ZLIB_HOME}/lib"
  ], [
    ZLIB_CPPFLAGS=""
    ZLIB_LDFLAGS=""
  ])

ZLIB_LDFLAGS="${ZLIB_LDFLAGS} -lz"
  AC_SUBST(ZLIB_CPPFLAGS)
AC_SUBST(ZLIB_LDFLAGS)

PTHREAD_LIBS=-lpthread
AX_PTHREAD([], [
  AC_MSG_ERROR([Could not find posix thread library.])
])


nitpick=false
AC_ARG_ENABLE([nitpicking],
 AS_HELP_STRING([--enable-nitpicking],[make compiler warn about possible problems]),
 [ test "$enableval" = "no" || nitpick=true ]
)
AM_CONDITIONAL(NITPICK, test x$nitpick = xtrue)

parallelize=false
AC_ARG_ENABLE([parallelization],
  AS_HELP_STRING([--enable-parallelization],[enable openmp]),
  [ test "$enableval" = "no" || parallelize=true ])
AM_CONDITIONAL(PARALLELIZE, test x$parallelize = xtrue)

profile=false
AC_ARG_ENABLE([profiling],
  AS_HELP_STRING([--enable-profiling], [add -pg to C++ compiler flags]),
  [ test "$enableval" = "no" || profile=true ])
AM_CONDITIONAL(PROFILE, test x$profile = xtrue)

vwbug=false
AC_ARG_ENABLE([debug],
  AS_HELP_STRING([--enable-debug], [Enable debugging, disable optimization in the compiler]),
  [ test "$enableval" = "no" || vwbug=true ])
AM_CONDITIONAL(VWBUG, test x$vwbug = xtrue)

clang_libcxx=false
AC_ARG_ENABLE([libc++],
  AS_HELP_STRING([--enable-libc++],
                 [Use Clang libc++ vs. GNU libstdc++ headers and libraries]),
  [ test "$enableval" = "no" || clang_libcxx=true])

AS_IF([test "x${clang_libcxx}" = xfalse],
  [
    AS_CASE([${host_os}], 
      [darwin*], [
        dnl Mac OSX: Really prefer libc++ when using clang++
        AS_CASE(["${CXX}"],
          [*clang++*], [
            clang_libcxx=true
            AC_MSG_NOTICE([**** clang++ compiler enables libc++ for Mac OS X.])
          ])
      ])
  ])

AM_CONDITIONAL([CLANG_LIBCXX], test x$clang_libcxx = xtrue)

pythonmod=false
AC_ARG_ENABLE([python],
  AS_HELP_STRING([--enable-python],
                 [Build the Vowpal Wabbit Python module; a specific version of the Python interpreter can be
                  enabled via the PYTHON_VERSION variable. [default=no]]),
  [ 
    AS_IF([test "${enableval}" != "no"],
      [
        pythonmod=true
        AM_PATH_PYTHON
        AX_PYTHON_DEVEL
        AX_BOOST_PYTHON
      ])
  ],
  [
    AC_MSG_NOTICE([**** Python module not enabled.])
  ])

AM_CONDITIONAL(PYTHON_MODULE, test x${pythonmod} = xtrue)

javajnimod=false
AC_ARG_ENABLE([java-jni],
  AS_HELP_STRING([--enable-java-jni],
                 [Build the Java JNI library. [default=no]]),
  [
    AS_IF([test "${enableval}" != "no"],
      [
        javajnimod=true
        AX_JAVA_OPTIONS
        AX_PROG_JAVA
        AX_PROG_JAVAC
        AX_JNI_INCLUDE_DIR
      ])
  ],
  [
    AC_MSG_NOTICE([**** Java JNI module not enabled.])
  ])

AM_CONDITIONAL([JAVA_JNI_MODULE], test "x${javajnimod}" = xtrue)

CXXFLAGS_JNI_INCLUDES=""
for d in ${JNI_INCLUDE_DIRS}; do
  CXXFLAGS_JNI_INCLUDES="${CXXFLAGS_JNI_INCLUDES} -I${d}"
done
AC_SUBST(CXXFLAGS_JNI_INCLUDES)

AS_CASE([$host_os],
  [freebsd*], [freebsd=true],
	[freebsd=false])
AM_CONDITIONAL(FREEBSD, test x$freebsd = xtrue)

dnl Extra code appended to config.h:
AH_BOTTOM([
#if defined(HAVE_CXX11) && HAVE_CXX11
/* If c++11 is available, then so is c++0x: */
#define HAVE_CXX0X 1
#endif
])

AC_CONFIG_FILES([
 Makefile
 vowpalwabbit/Makefile
 cluster/Makefile
 library/Makefile
 python/Makefile
 java/Makefile
])
AC_OUTPUT

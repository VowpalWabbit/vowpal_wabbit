AC_INIT([vowpal_wabbit], [6.1.4], [jl@hunch.net])
AC_DEFINE([PACKAGE_URL],["https://github.com/JohnLangford/vowpal_wabbit"],[project url])
AM_CONFIG_HEADER(vowpalwabbit/config.h)
AM_INIT_AUTOMAKE()

AC_CONFIG_MACRO_DIR([acinclude.d])

# if CXXFLAGS is not given by user, don't use default of "-O2 -g"
: ${CXXFLAGS=""}

AC_PROG_CXX
AC_PROG_CXXCPP
AC_LANG_CPLUSPLUS
AC_CANONICAL_HOST
AC_PROG_INSTALL
AM_PROG_LIBTOOL

AC_HEADER_STDC

AX_BOOST_BASE([1.0], [], [
  AC_MSG_ERROR([Could not find a valid version of boost.])
])
AX_BOOST_PROGRAM_OPTIONS()

AX_CHECK_ZLIB()
ZLIB_CPPFLAGS="-I${ZLIB_HOME}/include"
ZLIB_LDFLAGS="-L${ZLIB_HOME}/lib -lz"
AC_SUBST(ZLIB_CPPFLAGS)
AC_SUBST(ZLIB_LDFLAGS)

AX_PTHREAD([], [
  AC_MSG_ERROR([Could not find posix thread library.])
])


nitpick=false
AC_ARG_ENABLE([nitpicking],
 AC_HELP_STRING([--enable-nitpicking],[make compiler warn about possible problems]),
 [ test "$enableval" = "no" || nitpick=true ]
)
AM_CONDITIONAL(NITPICK, test x$nitpick = xtrue)

parallelize=false
AC_ARG_ENABLE([parallelization],AC_HELP_STRING([--enable-parallelization],[enable openmp]),[ test "$enableval" = "no" || parallelize=true ])
AM_CONDITIONAL(PARALLELIZE, test x$parallelize = xtrue)

profile=false
AC_ARG_ENABLE([profiling],AC_HELP_STRING([--enable-profiling],[add -pg -g CPPFLAGS]),[ test "$enableval" = "no" || profile=true ])
AM_CONDITIONAL(PROFILE, test x$profile = xtrue)

case $host in
     *-*-freebsd*)
	freebsd=true
	;;
     *)
	freebsd=false
	;;
esac
AM_CONDITIONAL(FREEBSD, test x$freebsd = xtrue)


AC_CONFIG_FILES([
 Makefile
 vowpalwabbit/Makefile
 cluster/Makefile
 library/Makefile
])
AC_OUTPUT

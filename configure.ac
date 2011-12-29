AC_INIT([vowpal_wabbit], [6.1], [jl@hunch.net])
AC_DEFINE([PACKAGE_URL],["https://github.com/JohnLangford/vowpal_wabbit"],[project url])
AM_CONFIG_HEADER(src/config.h)
AM_INIT_AUTOMAKE()

AC_CONFIG_MACRO_DIR([acinclude.d])

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

# make go fast
CPPFLAGS="$CPPFLAGS -O3 -fomit-frame-pointer -ffast-math -fno-strict-aliasing -D_FILE_OFFSET_BITS=64"

nitpick=false
AC_ARG_ENABLE([nitpicking],
 AC_HELP_STRING([--enable-nitpicking],[make compiler warn about possible problems]),
 [ test "$enableval" = "no" || nitpick=true ]
)
if $nitpick ; then
 CPP_NITPICK="-Wall -Wextra -Wundef -Wshadow -Wunsafe-loop-optimizations -Wconversion -Wmissing-format-attribute"
 CPP_NITPICK="$CPP_NITPICK -Wredundant-decls -ansi -Wmissing-noreturn"
 CPPFLAGS="$CPPFLAGS $CPP_NITPICK"
fi

parallelize=false
AC_ARG_ENABLE([parallelization],AC_HELP_STRING([--enable-parallelization],[enable openmp]),[ test "$enableval" = "no" || parallelize=true ])
if $parallelize ; then
  CPPFLAGS="$CPPCLAGS -Wno-strict-aliasing -fopenmp"
fi

profile=false
AC_ARG_ENABLE([profiling],AC_HELP_STRING([--enable-profiling],[add -pg -g CPPFLAGS]),[ test "$enableval" = "no" || profile=true ])
if $profile ; then
  CPPFLAGS="$CPPCLAGS -pg -g"
fi

AC_CONFIG_FILES([
 Makefile
 src/Makefile
 cluster/Makefile
])
AC_OUTPUT

# ===========================================================================
#      http://www.gnu.org/software/autoconf-archive/ax_boost_python.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_BOOST_PYTHON
#
# DESCRIPTION
#
#   This macro checks to see if the Boost.Python library is installed. It
#   also attempts to guess the correct library name using several attempts.
#   It tries to build the library name using a user supplied name or suffix
#   and then just the raw library.
#
#   If the library is found, HAVE_BOOST_PYTHON is defined and
#   BOOST_PYTHON_LIB is set to the name of the library.
#
#   This macro calls AC_SUBST(BOOST_PYTHON_LIB).
#
#   In order to ensure that the Python headers and the Boost libraries are
#   specified on the include path, this macro requires AX_PYTHON_DEVEL and
#   AX_BOOST_BASE to be called.
#
# LICENSE
#
#   Copyright (c) 2008 Michael Tindal
#   Copyright (c) 2013 Daniel M"ullner <daniel@danifold.net>
#
#   This program is free software; you can redistribute it and/or modify it
#   under the terms of the GNU General Public License as published by the
#   Free Software Foundation; either version 2 of the License, or (at your
#   option) any later version.
#
#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
#   Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program. If not, see <http://www.gnu.org/licenses/>.
#
#   As a special exception, the respective Autoconf Macro's copyright owner
#   gives unlimited permission to copy, distribute and modify the configure
#   scripts that are the output of Autoconf when processing the Macro. You
#   need not follow the terms of the GNU General Public License when using
#   or distributing such scripts, even though portions of the text of the
#   Macro appear in them. The GNU General Public License (GPL) does govern
#   all other use of the material that constitutes the Autoconf Macro.
#
#   This special exception to the GPL applies to versions of the Autoconf
#   Macro released by the Autoconf Archive. When you make and distribute a
#   modified version of the Autoconf Macro, you may extend this special
#   exception to the GPL to apply to your modified version as well.

#serial 20

AC_DEFUN([AX_BOOST_PYTHON],
[AC_REQUIRE([AX_PYTHON_DEVEL])dnl
AC_REQUIRE([AX_BOOST_BASE])dnl
AC_LANG_PUSH([C++])
ax_boost_python_save_CPPFLAGS="$CPPFLAGS"
ax_boost_python_save_LDFLAGS="$LDFLAGS"
ax_boost_python_save_LIBS="$LIBS"
if test "x$PYTHON_CPPFLAGS" != "x"; then
  CPPFLAGS="$PYTHON_CPPFLAGS $CPPFLAGS"
fi
if test "x$PYTHON_LDFLAGS" != "x"; then
  LDFLAGS="$PYTHON_LDFLAGS $LDFLAGS"
fi
if test "x$BOOST_CPPFLAGS" != "x"; then
  CPPFLAGS="$BOOST_CPPFLAGS $CPPFLAGS"
fi
if test "x$BOOST_LDFLAGS" != "x"; then
  LDFLAGS="$BOOST_LDFLAGS $LDFLAGS"
fi
AC_CACHE_CHECK(whether the Boost::Python library is available,
ac_cv_boost_python,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <boost/python/module.hpp>
BOOST_PYTHON_MODULE(test) { throw "Boost::Python test."; }]], [])],
    ac_cv_boost_python=yes, ac_cv_boost_python=no)
])
if test "$ac_cv_boost_python" = "yes"; then
  AC_DEFINE(HAVE_BOOST_PYTHON,,[define if the Boost::Python library is available])
  ax_python_lib=boost_python
  AC_ARG_WITH([boost-python],AS_HELP_STRING([--with-boost-python],[specify yes/no or the boost python library or suffix to use]),
  [if test "x$with_boost_python" != "xno" -a "x$with_boost_python" != "xyes"; then
     ax_python_lib=$with_boost_python
     ax_boost_python_lib=boost_python-$with_boost_python
   fi])
  BOOSTLIBDIR=`echo $BOOST_LDFLAGS | sed -e 's/@<:@^\/@:>@*//'`
  for ax_lib in $ax_python_lib $ax_boost_python_lib `ls $BOOSTLIBDIR/libboost_python*.so* $BOOSTLIBDIR/libboost_python*.dylib* $BOOSTLIBDIR/libboost_python*.a* 2>/dev/null | sed 's,.*/,,' | sed -e 's;^lib\(boost_python.*\)\.so.*$;\1;' -e 's;^lib\(boost_python.*\)\.dylib.*$;\1;' -e 's;^lib\(boost_python.*\)\.a.*$;\1;' ` boost_python boost_python3; do
    AS_VAR_PUSHDEF([ax_Lib], [ax_cv_lib_$ax_lib''_BOOST_PYTHON_MODULE])dnl
    AC_CACHE_CHECK([whether $ax_lib is the correct library], [ax_Lib],
    [LIBS="-l$ax_lib $ax_boost_python_save_LIBS"
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <boost/python/module.hpp>
BOOST_PYTHON_MODULE(test) { throw "Boost::Python test."; }]], [])],
        [AS_VAR_SET([ax_Lib], [yes])],
        [AS_VAR_SET([ax_Lib], [no])])])
    AS_VAR_IF([ax_Lib], [yes], [BOOST_PYTHON_LIB=$ax_lib break], [])
    AS_VAR_POPDEF([ax_Lib])dnl
  done
  AC_SUBST(BOOST_PYTHON_LIB)
fi
CPPFLAGS="$ax_boost_python_save_CPPFLAGS"
LDFLAGS="$ax_boost_python_save_LDFLAGS"
LIBS="$ax_boost_python_save_LIBS"
AC_LANG_POP([C++])
])dnl

#!/bin/sh

# Ensure that both CXX and CC are set. Default to GNU and prefer
# Clang, if available.
if test -z "${CXX}" -a -z "${CC}"; then
  if test -x "`which g++`" ; then
    CXX=g++
    CC=gcc
  fi

  if test -x "`which clang++`" ; then
    CXX=clang++
    CC=clang
  fi
else
  test -z "${CXX}" && {
    echo "CC is set, but CXX isn't: Must set both CXX and CC" 1>&2
    exit 1
  }
  test -z "${CC}" && {
    echo "CXX is set, but CXX isn't: Must set both CXX and CC" 1>&2
    exit 1
  }
fi

case $( uname -s ) in
 Darwin)
  alias vwlibtool=glibtoolize
  if [ -z $AC_PATH ]; then
    if [ -d /opt/local/share ]; then
      AC_PATH="/opt/local/share"
    else
      AC_PATH="/usr/local/share"
    fi
  fi
  ;;
 Linux)
  AC_PATH=/usr/share
  ldconfig=""
  # Do not assume that user's path contains the usual places where ldconfig
  # is installed.
  for p in /sbin /usr/sbin `echo ${PATH} | sed 's/:/ /g'`; do
    if test -x ${p}/ldconfig; then
      ldconfig=${p}/ldconfig
      break
    fi
  done
  if test "x${ldconfig}" = x; then
    ldconfig=ldconfig
  fi
  LIBFILE=`${ldconfig} -p | grep program_options | tail -n 1 | cut -d '>' -f 2`
  echo "Boost at: $LIBFILE"
  BOOST_DIR_ARG="--with-boost-libdir=`dirname $LIBFILE`"
  echo "Using $BOOST_DIR_ARG"
  alias vwlibtool=libtoolize
  ;;
 *)
  alias vwlibtool=libtoolize
  ${AC_PATH:=/usr/share}
  ;;
esac

vwlibtool -f -c && aclocal -I ./acinclude.d -I $AC_PATH/aclocal && autoheader && touch README && automake -ac -Woverride && autoconf && ./configure "$@" $BOOST_DIR_ARG CXX=${CXX} CC=${CC}

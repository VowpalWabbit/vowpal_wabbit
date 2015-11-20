#!/bin/sh

if [ -z $CXX ]; then
if [ -x "`which g++`" ]; then CXX=g++
elif [ -x "`which clang++`" ]; then CXX=clang++
fi
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
  for p in `echo ${PATH} | sed 's/:/ /g'` /sbin /usr/sbin; do
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

vwlibtool -f -c && aclocal -I ./acinclude.d -I $AC_PATH/aclocal && autoheader && touch README && automake -ac -Woverride && autoconf && ./configure "$@" $BOOST_DIR_ARG CXX=$CXX

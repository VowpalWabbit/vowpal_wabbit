#!/bin/sh
AC_PATH=/usr/share
[ -x "`which g++`" ] && CXX=g++
[ -x "`which clang++`" ] && CXX=clang++
case $( uname -s ) in
 Darwin)  alias vwlibtool=glibtoolize
	AC_PATH=/opt/local/share;;
 *)	alias vwlibtool=libtoolize;;
esac

vwlibtool -f -c && aclocal -I ./acinclude.d -I $AC_PATH/aclocal && autoheader && touch README && automake -ac -Woverride && autoconf && ./configure "$@" CXX=$CXX

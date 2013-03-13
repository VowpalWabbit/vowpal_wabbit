#! /bin/sh
libtoolize -f -c && aclocal -I ./acinclude.d -I /usr/share/aclocal && autoheader && automake -ac -Woverride && autoconf && ./configure "$@"

bin_PROGRAMS = library_example

library_example_SOURCES = library_example.cc
library_example_LDADD = ../vowpalwabbit/libvw.la ../vowpalwabbit/liballreduce.la

ACLOCAL_AMFLAGS = -I acinclude.d

INCLUDES = ${BOOST_CPPFLAGS} ${ZLIB_CPPFLAGS} ${PTHREAD_CFLAGS}
AM_LDFLAGS = ${BOOST_LDFLAGS} ${BOOST_PROGRAM_OPTIONS_LIB} ${ZLIB_LDFLAGS} ${PTHREAD_LIBS}

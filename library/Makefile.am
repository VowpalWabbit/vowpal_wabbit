bin_PROGRAMS = library_example ezexample_train ezexample_predict
EXAMPLE_LIBS = ../vowpalwabbit/libvw.la 

library_example_SOURCES = library_example.cc
library_example_LDADD = ../vowpalwabbit/libvw.la 

ezexample_train_SOURCES = ezexample_train.cc
ezexample_train_LDADD = ${EXAMPLE_LIBS}

ezexample_predict_SOURCES = ezexample_predict.cc
ezexample_predict_LDADD = ${EXAMPLE_LIBS}

ACLOCAL_AMFLAGS = -I acinclude.d

AM_CXXFLAGS = ${BOOST_CPPFLAGS} ${ZLIB_CPPFLAGS} ${PTHREAD_CFLAGS}
AM_LDFLAGS = ${BOOST_LDFLAGS} ${BOOST_PROGRAM_OPTIONS_LIB} ${ZLIB_LDFLAGS} ${PTHREAD_LIBS}

if CLANG_LIBCXX
AM_CXXFLAGS += -stdlib=libc++
endif

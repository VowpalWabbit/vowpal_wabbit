bin_PROGRAMS = library_example ezexample_train ezexample_predict
EXAMPLE_LIBS = ../vowpalwabbit/libvw.la ../vowpalwabbit/liballreduce.la
EXAMPLE_DEPS = ../vowpalwabbit/libvw.la ../vowpalwabbit/liballreduce.la

library_example_SOURCES = library_example.cc
library_example_LDADD = ${EXAMPLE_LIBS}
library_example_DEPENDENCIES = ${EXAMPLE_DEPS}

ezexample_train_SOURCES = ezexample_train.cc
ezexample_train_LDADD = ${EXAMPLE_LIBS}
ezexample_train_DEPENDENCIES = ${EXAMPLE_DEPS}

ezexample_predict_SOURCES = ezexample_predict.cc
ezexample_predict_LDADD = ${EXAMPLE_LIBS}
ezexample_predict_DEPENDENCIES = ${EXAMPLE_DEPS}

ACLOCAL_AMFLAGS = -I acinclude.d

AM_CXXFLAGS = ${BOOST_CPPFLAGS} ${ZLIB_CPPFLAGS} ${PTHREAD_CFLAGS}

AM_CXXFLAGS += -I ../rapidjson/include

AM_LDFLAGS = ${BOOST_LDFLAGS} ${BOOST_PROGRAM_OPTIONS_LIB} ${ZLIB_LDFLAGS} ${PTHREAD_LIBS}

if CLANG_LIBCXX
AM_CXXFLAGS += -stdlib=libc++
endif

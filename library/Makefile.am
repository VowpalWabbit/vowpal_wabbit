noinst_PROGRAMS = library_example ezexample_predict ezexample_train recommend gd_mf_weights test_search

ACLOCAL_AMFLAGS = -I acinclude.d

AM_CXXFLAGS = ${BOOST_CPPFLAGS} ${ZLIB_CPPFLAGS} ${PTHREAD_CFLAGS}
AM_LDFLAGS = ${BOOST_LDFLAGS} ${BOOST_PROGRAM_OPTIONS_LIB} ${ZLIB_LDFLAGS} ${PTHREAD_LIBS}

if CLANG_LIBCXX
AM_CXXFLAGS += -stdlib=libc++
endif

noinst_HEADERS = libsearch.h

library_example_SOURCES = library_example.cc
library_example_LDADD = ../vowpalwabbit/libvw.la ../vowpalwabbit/liballreduce.la

ezexample_predict_SOURCES = ezexample_predict.cc
ezexample_predict_LDADD = ../vowpalwabbit/libvw.la ../vowpalwabbit/liballreduce.la

ezexample_train_SOURCES = ezexample_train.cc
ezexample_train_LDADD = ../vowpalwabbit/libvw.la ../vowpalwabbit/liballreduce.la

recommend_SOURCES = recommend.cc
recommend_LDADD = ../vowpalwabbit/libvw.la ../vowpalwabbit/liballreduce.la

gd_mf_weights_SOURCES = gd_mf_weights.cc
gd_mf_weights_LDADD = ../vowpalwabbit/libvw.la ../vowpalwabbit/liballreduce.la

test_search_SOURCES = test_search.cc
test_search_LDADD = ../vowpalwabbit/libvw.la ../vowpalwabbit/liballreduce.la

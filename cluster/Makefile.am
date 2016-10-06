bin_PROGRAMS = spanning_tree

spanning_tree_SOURCES = spanning_tree_main.cc ../vowpalwabbit/spanning_tree.cc ../vowpalwabbit/vw_exception.cc

spanning_tree_LDADD = ../vowpalwabbit/liballreduce.la
spanning_tree_DEPENDENCIES = ../vowpalwabbit/liballreduce.la

ACLOCAL_AMFLAGS = -I acinclude.d

AM_CPPFLAGS = -I$(top_srcdir)/vowpalwabbit
AM_CXXFLAGS = ${BOOST_CPPFLAGS} ${ZLIB_CPPFLAGS} ${PTHREAD_CFLAGS}
AM_LDFLAGS = ${BOOST_LDFLAGS} ${BOOST_PROGRAM_OPTIONS_LIB} ${ZLIB_LDFLAGS} ${PTHREAD_LIBS}

if CLANG_LIBCXX
AM_CXXFLAGS += -stdlib=libc++ -I ../vowpalwabbit -std=c++0x
endif

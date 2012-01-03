lib_LTLIBRARIES = libvowpalwabbit.la

libvowpalwabbit_la_SOURCES = hash.cc global_data.cc io.cc parse_regressor.cc parse_primitives.cc unique_sort.cc cache.cc \
	simple_label.cc parse_example.cc sparse_dense.cc network.cc parse_args.cc allreduce.cc accumulate.cc \
	gd.cc lda_core.cc gd_mf.cc bfgs.cc noop.cc parser.cc vw.cc loss_functions.cc sender.cc

bin_PROGRAMS = vw active_interactor

LIBDIR = ../vowpalwabbit

vw_SOURCES = main.cc
vw_LDFLAGS = -lvowpalwabbit -L$(LIBDIR)/.libs
vw_CPPFLAGS = -I..

active_interactor_SOURCES = active_interactor.cc
active_interactor_LDFLAGS = -lvowpalwabbit -L$(LIBDIR)/.libs
active_interactor_CPPFLAGS = -I..

ACLOCAL_AMFLAGS = -I acinclude.d

INCLUDES = ${BOOST_CPPFLAGS} ${ZLIB_CPPFLAGS} ${PTHREAD_CFLAGS}
AM_LDFLAGS = ${BOOST_LDFLAGS} ${BOOST_PROGRAM_OPTIONS_LIB} ${ZLIB_LDFLAGS} ${PTHREAD_LIBS}

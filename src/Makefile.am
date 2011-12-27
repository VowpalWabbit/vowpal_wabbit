bin_PROGRAMS = vw active_interactor

vw_SOURCES = hash.cc global_data.cc io.cc parse_regressor.cc parse_primitives.cc unique_sort.cc cache.cc \
	simple_label.cc parse_example.cc sparse_dense.cc network.cc parse_args.cc allreduce.cc accumulate.cc \
	gd.cc lda_core.cc gd_mf.cc bfgs.cc noop.cc parser.cc vw.cc loss_functions.cc sender.cc main.cc

active_interactor_SOURCES = active_interactor.cc

ACLOCAL_AMFLAGS = -I acinclude.d

INCLUDES = ${BOOST_CPPFLAGS} ${ZLIB_CPPFLAGS} ${PTHREAD_CFLAGS}
AM_LDFLAGS = ${BOOST_LDFLAGS} ${BOOST_PROGRAM_OPTIONS_LIB} ${ZLIB_LDFLAGS} ${PTHREAD_LIBS}

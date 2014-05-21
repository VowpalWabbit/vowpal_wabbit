lib_LTLIBRARIES = liballreduce.la libvw.la

liballreduce_la_SOURCES = allreduce.cc

bin_PROGRAMS = vw active_interactor

libvw_la_SOURCES = hash.cc memory.cc global_data.cc io_buf.cc parse_regressor.cc parse_primitives.cc unique_sort.cc cache.cc rand48.cc simple_label.cc multiclass.cc oaa.cc ect.cc autolink.cc binary.cc lrq.cc cost_sensitive.cc csoaa.cc cb.cc cb_algs.cc wap.cc searn.cc searn_sequencetask.cc parse_example.cc scorer.cc sparse_dense.cc network.cc parse_args.cc accumulate.cc gd.cc learner.cc lda_core.cc gd_mf.cc mf.cc bfgs.cc noop.cc print.cc example.cc parser.cc loss_functions.cc sender.cc nn.cc bs.cc cbify.cc topk.cc stagewise_poly.cc stagewise_poly_ssm.cc

# accumulate.cc uses all_reduce
libvw_la_LIBADD = liballreduce.la

vw_SOURCES = main.cc
vw_CXXFLAGS = $(AM_CXXFLAGS)

vw_LDADD = libvw.la liballreduce.la
vw_DEPENDENCIES = libvw.la liballreduce.la

active_interactor_SOURCES = active_interactor.cc

ACLOCAL_AMFLAGS = -I acinclude.d

INCLUDES = ${BOOST_CPPFLAGS} ${ZLIB_CPPFLAGS} ${PTHREAD_CFLAGS}
AM_LDFLAGS = ${BOOST_LDFLAGS} ${BOOST_PROGRAM_OPTIONS_LIB} ${ZLIB_LDFLAGS} ${PTHREAD_LIBS}

if PROFILE
AM_CXXFLAGS = -pg -g -D_FILE_OFFSET_BITS=64
else
AM_CXXFLAGS = -O3 -fomit-frame-pointer -ffast-math -fno-strict-aliasing -D_FILE_OFFSET_BITS=64
endif

if NITPICK
AM_CXXFLAGS += -Wextra -Wundef -Wshadow -Wunsafe-loop-optimizations -Wconversion -Wmissing-format-attribute
AM_CXXFLAGS += -Wredundant-decls -ansi -Wmissing-noreturn
endif

if PARALLELIZE
AM_CXXFLAGS += -Wno-strict-aliasing -fopenmp
endif

if FREEBSD
AM_CXXFLAGS += -l compat -Wall
else
AM_CXXFLAGS += -Wall -pedantic
endif


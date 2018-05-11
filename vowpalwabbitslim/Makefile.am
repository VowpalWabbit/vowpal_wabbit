libvwslim_la_SOURCES = ../vowpalwabbit/example_predict.cc example_predict_builder.cc model_parser.cc opts.cc vw_slim_predict.cc

ACLOCAL_AMFLAGS = -I acinclude.d

AM_CXXFLAGS = -Wall -Wno-unused-local-typedefs

if CLANG_LIBCXX
AM_CXXFLAGS += -stdlib=libc++
endif

AM_CXXFLAGS += -I ../vowpalwabbit -I ../explore

AM_CXXFLAGS += $(CXXOPTIMIZE)


bin_PROGRAMS = vw_slim_predict

vw_slim_predict_SOURCES = ../vowpalwabbit/vw_exception.cc ../vowpalwabbit/example_predict.cc main_predict.cc vw_slim_predict.cc

ACLOCAL_AMFLAGS = -I acinclude.d

AM_CXXFLAGS = -Wall -Wno-unused-local-typedefs
LIBS = 
CXXOPTIMIZE =

if PROFILE
CXXOPTIMIZE += -pg
endif

if VWBUG
CXXOPTIMIZE += -g -O1
else
CXXOPTIMIZE += -O3 -fomit-frame-pointer -DNDEBUG
endif

if NITPICK
AM_CXXFLAGS += -Wextra -Wundef -Wshadow -Wunsafe-loop-optimizations -Wconversion -Wmissing-format-attribute
AM_CXXFLAGS += -Wredundant-decls -ansi -Wmissing-noreturn
endif

if PARALLELIZE
AM_CXXFLAGS += -Wno-strict-aliasing -fopenmp
endif

if FREEBSD
AM_CXXFLAGS += -l compat
else
AM_CXXFLAGS += -pedantic
endif

if CLANG_LIBCXX
AM_CXXFLAGS += -stdlib=libc++
endif

AM_CXXFLAGS += $(CXXOPTIMIZE)

vw_slim_predict_CXXFLAGS = $(AM_CXXFLAGS)

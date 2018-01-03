CXX = $(shell which g++)
# -- if you want to test 32-bit use this instead,
#    it sometimes reveals type portability issues
# CXX = $(shell which g++) -m32
ifneq ($(CXX),)
  #$(warning Using clang: "$(CXX)")
  ARCH = -D__extern_always_inline=inline
else
  CXX = clang++
  $(warning Using clang++)
endif
#ARCH = $(shell test `$CXX -v 2>&1 | tail -1 | cut -d ' ' -f 3 | cut -d '.' -f 1,2` \< 4.3 && echo -march=nocona || echo -march=native)

ifeq ($(CXX),)
  $(warning No compiler found)
  exit 1
endif

UNAME := $(shell uname)
LIBS = -l boost_program_options -l pthread -l z
BOOST_INCLUDE = -I /usr/include
BOOST_LIBRARY = -L /usr/local/lib -L /usr/lib
NPROCS := 1

ifeq ($(UNAME), Linux)
  BOOST_LIBRARY += -L /usr/lib/x86_64-linux-gnu
  NPROCS:=$(shell grep -c ^processor /proc/cpuinfo)
endif
ifeq ($(UNAME), FreeBSD)
  LIBS = -l boost_program_options -l pthread -l z -l compat
  BOOST_INCLUDE = -I /usr/local/include
  NPROCS:=$(shell grep -c ^processor /proc/cpuinfo)
endif
ifeq "CYGWIN" "$(findstring CYGWIN,$(UNAME))"
  LIBS = -l boost_program_options-mt -l pthread -l z
  BOOST_INCLUDE = -I /usr/include
  NPROCS:=$(shell grep -c ^processor /proc/cpuinfo)
endif
ifeq ($(UNAME), Darwin)
  LIBS = -lboost_program_options-mt -lboost_serialization-mt -l pthread -l z
  # On Macs, the location isn't always clear
  #	brew uses /usr/local
  #	but /opt/local seems to be preferred by some users
  #	so we try them both
  ifneq (,$(wildcard /usr/local/include))
    BOOST_INCLUDE = -I /usr/local/include
    BOOST_LIBRARY = -L /usr/local/lib
  endif
  ifneq (,$(wildcard /opt/local/include))
    BOOST_INCLUDE = -I /opt/local/include
    BOOST_LIBRARY = -L /opt/local/lib
  endif
  NPROCS:=$(shell sysctl -n hw.ncpu)
endif

JSON_INCLUDE = -I ../rapidjson/include

#LIBS = -l boost_program_options-gcc34 -l pthread -l z
OPTIM_FLAGS ?= -DNDEBUG -O3 -fomit-frame-pointer -fno-strict-aliasing #-ffast-math #uncomment for speed, comment for testability
ifeq ($(UNAME), FreeBSD)
  WARN_FLAGS = -Wall
else
  WARN_FLAGS = -Wall -pedantic
endif

# for normal fast execution.
FLAGS = -std=c++0x $(CFLAGS) $(LDFLAGS) $(ARCH) $(WARN_FLAGS) $(OPTIM_FLAGS) -D_FILE_OFFSET_BITS=64 $(BOOST_INCLUDE) $(JSON_INCLUDE) -fPIC #-DVW_LDA_NO_SSE

# for profiling -- note that it needs to be gcc
#FLAGS = -std=c++0x $(CFLAGS) $(LDFLAGS) $(ARCH) $(WARN_FLAGS) -O2 -fno-strict-aliasing -D_FILE_OFFSET_BITS=64 $(BOOST_INCLUDE) $(JSON_INCLUDE) -pg  -fPIC
#CXX = g++

# for valgrind / gdb debugging
#FLAGS = -std=c++0x $(CFLAGS) $(LDFLAGS) $(ARCH) $(WARN_FLAGS) -D_FILE_OFFSET_BITS=64 $(BOOST_INCLUDE) $(JSON_INCLUDE) -g -O0  -fPIC

# for valgrind profiling: run 'valgrind --tool=callgrind PROGRAM' then 'callgrind_annotate --tree=both --inclusive=yes'
#FLAGS = -std=c++0x $(CFLAGS) $(LDFLAGS) -Wall $(ARCH) -ffast-math -D_FILE_OFFSET_BITS=64 $(BOOST_INCLUDE) $(JSON_INCLUDE)  -g -fomit-frame-pointer -ffast-math -fno-strict-aliasing  -fPIC

FLAGS += -I ../rapidjson/include

BINARIES = vw active_interactor
MANPAGES = vw.1

default:	vw

all:	vw library_example java spanning_tree

%.1:	%
	help2man --no-info --name="Vowpal Wabbit -- fast online learning tool" ./$< > $@

export

spanning_tree:
	cd cluster; $(MAKE)

vw:
	cd vowpalwabbit; $(MAKE) -j $(NPROCS) things

#Target-specific flags for a profiling build.  (Copied from line 70)
vw_gcov: FLAGS = -std=c++0x $(CFLAGS) $(LDFLAGS) $(ARCH) $(WARN_FLAGS) -g -O0 -fprofile-arcs -ftest-coverage -fno-strict-aliasing -D_FILE_OFFSET_BITS=64 $(BOOST_INCLUDE) $(JSON_INCLUDE) -pg  -fPIC #-DVW_LDA_NO_S
vw_gcov: CXX = g++
vw_gcov:
	cd vowpalwabbit && env LDFLAGS="-fprofile-arcs -ftest-coverage -lgcov"; $(MAKE) -j $(NPROCS) things

active_interactor:
	cd vowpalwabbit; $(MAKE)

library_example: vw
	cd library; $(MAKE) -j $(NPROCS) things

#Target-specific flags for a profiling build.  (Copied from line 70)
library_example_gcov: FLAGS = -std=c++0x $(CFLAGS) $(LDFLAGS) $(ARCH) $(WARN_FLAGS) -g -O0 -fprofile-arcs -ftest-coverage -fno-strict-aliasing -D_FILE_OFFSET_BITS=64 $(BOOST_INCLUDE) $(JSON_INCLUDE) -pg  -fPIC #-DVW_LDA_NO_S
library_example_gcov: CXX = g++
library_example_gcov: vw_gcov
	cd library && env LDFLAGS="-fprofile-arcs -ftest-coverage -lgcov"; $(MAKE) things

python: vw
	cd python; $(MAKE) things

java: vw
	cd java; $(MAKE) things

.FORCE:

test: .FORCE vw library_example
	@echo "vw running test-suite..."
	(cd test && ./RunTests -d -fe -E 0.001 ../vowpalwabbit/vw)

test_gcov: .FORCE vw_gcov library_example_gcov
	@echo "vw running test-suite..."
	(cd test && ./RunTests -d -fe -E 0.001 ../vowpalwabbit/vw ../vowpalwabbit/vw)

bigtests:	.FORCE vw
	(cd big_tests && $(MAKE) $(MAKEFLAGS))

install: $(BINARIES)
	cd vowpalwabbit; cp $(BINARIES) /usr/local/bin; cd ../cluster; $(MAKE) install; cd ../java; $(MAKE) install;

doc:
	(cd doc && doxygen Doxyfile)

clean:
	cd vowpalwabbit && $(MAKE) clean
	cd cluster && $(MAKE) clean
	cd library && $(MAKE) clean
	cd python  && $(MAKE) clean
	cd java    && $(MAKE) clean

.PHONY: all clean install doc

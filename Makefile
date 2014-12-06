#CXX = $(shell which clang++)
# -- if you want to test 32-bit use this instead,
#    it sometimes reveals type portability issues
# CXX = $(shell which clang++) -m32
ifneq ($(CXX),)
  #$(warning Using clang: "$(CXX)")
  ARCH = -D__extern_always_inline=inline
else
  CXX = g++
  $(warning Using g++)
ARCH = $(shell test `g++ -v 2>&1 | tail -1 | cut -d ' ' -f 3 | cut -d '.' -f 1,2` \< 4.3 && echo -march=nocona || echo -march=native)
endif

ifeq ($(CXX),)
  $(warning No compiler found)
  exit 1
endif

UNAME := $(shell uname)
LIBS = -l boost_program_options -l pthread -l z
BOOST_INCLUDE = -I /usr/include
BOOST_LIBRARY = -L /usr/lib

ifeq ($(UNAME), Linux)
  BOOST_LIBRARY += -L /usr/lib/x86_64-linux-gnu
endif
ifeq ($(UNAME), FreeBSD)
  LIBS = -l boost_program_options -l pthread -l z -l compat
  BOOST_INCLUDE = -I /usr/local/include
endif
ifeq "CYGWIN" "$(findstring CYGWIN,$(UNAME))"
  LIBS = -l boost_program_options-mt -l pthread -l z
  BOOST_INCLUDE = -I /usr/include
endif
ifeq ($(UNAME), Darwin)
  LIBS = -lboost_program_options-mt -lboost_serialization-mt -l pthread -l z
  # On Macs, the location isn't always clear
  #	brew uses /usr/local
  #	but /opt/local seems to be preferred by some users
  #	so we try them both
  ifneq (,$(wildcard /usr/local/include))
    BOOST_INCLUDE += -I /usr/local/include
    BOOST_LIBRARY += -L /usr/local/lib
  endif
  ifneq (,$(wildcard /opt/local/include))
    BOOST_INCLUDE += -I /opt/local/include
    BOOST_LIBRARY += -L /opt/local/lib
  endif
endif

#LIBS = -l boost_program_options-gcc34 -l pthread -l z

OPTIM_FLAGS = -O3 -fomit-frame-pointer -fno-strict-aliasing #-ffast-math #uncomment for speed, comment for testability
ifeq ($(UNAME), FreeBSD)
  WARN_FLAGS = -Wall
else
  WARN_FLAGS = -Wall -pedantic
endif

# for normal fast execution.
FLAGS = -std=c++0x $(CFLAGS) $(LDFLAGS) $(ARCH) $(WARN_FLAGS) $(OPTIM_FLAGS) -D_FILE_OFFSET_BITS=64 -DNDEBUG $(BOOST_INCLUDE)  -fPIC #-DVW_LDA_NO_SSE

# for profiling -- note that it needs to be gcc
#FLAGS = $(CFLAGS) $(LDFLAGS) $(ARCH) $(WARN_FLAGS) -O2 -fno-strict-aliasing -ffast-math -D_FILE_OFFSET_BITS=64 $(BOOST_INCLUDE) -pg  -fPIC #-DVW_LDA_NO_S
#CXX = g++

# for valgrind / gdb debugging
#FLAGS = -std=c++0x $(CFLAGS) $(LDFLAGS) $(ARCH) $(WARN_FLAGS) -ffast-math -D_FILE_OFFSET_BITS=64 $(BOOST_INCLUDE) -g -O0  -fPIC

# for valgrind profiling: run 'valgrind --tool=callgrind PROGRAM' then 'callgrind_annotate --tree=both --inclusive=yes'
#FLAGS = $(CFLAGS) $(LDFLAGS) -Wall $(ARCH) -ffast-math -D_FILE_OFFSET_BITS=64 $(BOOST_INCLUDE) -g -O2 -fomit-frame-pointer -ffast-math -fno-strict-aliasing  -fPIC

BINARIES = vw active_interactor
MANPAGES = vw.1

all:	vw spanning_tree library_example python java

%.1:	%
	help2man --no-info --name="Vowpal Wabbit -- fast online learning tool" ./$< > $@

export

spanning_tree:
	cd cluster; $(MAKE)

vw:
	cd vowpalwabbit; $(MAKE) -j 8 things

active_interactor:
	cd vowpalwabbit; $(MAKE)

library_example: vw
	cd library; $(MAKE) things

python: vw
	cd python; $(MAKE) things
	
ifneq ($(JAVA_HOME),)
java: vw
	cd java; $(MAKE) things
endif

.FORCE:

test: .FORCE
	@echo "vw running test-suite..."
	(cd test && ./RunTests -d -fe -E 0.001 ../vowpalwabbit/vw ../vowpalwabbit/vw)

install: $(BINARIES)
	cd vowpalwabbit; cp $(BINARIES) /usr/local/bin; cd ../cluster; $(MAKE) install

clean:
	cd vowpalwabbit && $(MAKE) clean
	cd cluster && $(MAKE) clean
	cd library && $(MAKE) clean
	cd python  && $(MAKE) clean
ifneq ($(JAVA_HOME),)
	cd java    && $(MAKE) clean
endif

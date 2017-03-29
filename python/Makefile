#
# Critical variables are passed recursively (via the environment)
# by the top level Makefile when calling $(MAKE)
#
# If we don't know where to look for boost - it's a no go.
#
#ifeq ($(BOOST_LIBRARY),)
#  $(error Please run 'make' at the top level only)
#endif

# The python version and python boost library are resolved and set using setup.py
# When running make directly PYTHON_VERSION and PYTHON_LIBS env vars can be set to change these defaults
ifeq ($(PYTHON_VERSION),)
  PYTHON_VERSION = 2.7
endif

ifeq ($(PYTHON_LIBS),)
  PYTHON_LIBS = -l boost_python
endif

ifeq ($(VIRTUAL_ENV), )
  PYTHON_INCLUDE = $(shell python$(PYTHON_VERSION)-config --includes)
  PYTHON_LDFLAGS = $(shell python$(PYTHON_VERSION)-config --ldflags)
else
  PYTHON_INCLUDE = $(shell python-config --includes)
  PYTHON_LDFLAGS = $(shell python-config --ldflags)
endif

JSON_INCLUDE = -I ../rapidjson/include

ifeq (,$(wildcard ../vowpalwabbit/vw.so))
  VWLIBS = ../vowpalwabbit/libvw.a ../vowpalwabbit/liballreduce.a
else
  VWLIBS = -L ../vowpalwabbit -l vw -l allreduce
endif
PYLIBVW = pylibvw.so

all:
	cd ..; $(MAKE) python

things: $(PYLIBVW)

ifeq "CYGWIN" "$(findstring CYGWIN,$(UNAME))"
  ifeq ($(PYTHON_LIBS),)
    PYTHON_LIBS = -l boost_python-mt
  endif
  PYLIBVW = pylibvw.dll
endif
ifneq ($(UNAME), Darwin)
  EXPORT_DYNAMIC = -Wl,--export-dynamic
endif

STDLIBS = $(BOOST_LIBRARY) $(LIBS) $(PYTHON_LIBS)

$(PYLIBVW): pylibvw.o  ../vowpalwabbit/libvw.a
	$(CXX) -shared $(EXPORT_DYNAMIC) pylibvw.o $(BOOST_LIBRARY) $(PYTHON_LDFLAGS) $(VWLIBS) $(STDLIBS) -o $(PYLIBVW)

pylibvw.o: pylibvw.cc
	$(info Using Python $(PYTHON_VERSION))
	$(CXX) -std=c++0x $(PYTHON_INCLUDE) $(BOOST_INCLUDE) $(JSON_INCLUDE) -fPIC -c pylibvw.cc -o pylibvw.o

clean:
	rm -f *.o $(PYLIBVW)

.PHONY: all clean things

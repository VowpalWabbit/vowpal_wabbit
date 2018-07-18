#
# Critical variables are passed recursively (via the environment)
# by the top level Makefile when calling $(MAKE)
#
# If we don't know where to look for boost - it's a no go.
#
#ifeq ($(BOOST_LIBRARY),)
#  $(error Please run 'make' at the top level only)
#endif

# When using setup.py to install the PYTHON_* env vars are resolved and set automatically

# When running make directly PYTHON_VERSION and PYTHON_LIBS env vars can be set to change these defaults.
# For Python 3 one should find the version of libboost-python-dev installed locally as it may not be in
# sync with the python version. For example, on Ubuntu 14.04 apt-get installs libboost_python-py34.so.*
# and for that the setting required is PYTHON_LIBS="-l boost_python-py34".
ifeq ($(PYTHON_VERSION),)
  PYTHON_VERSION = 2.7
endif

ifeq ($(PYTHON_LIBS),)
  PYTHON_LIBS = -l boost_python
endif

ifeq ($(PYTHON_INCLUDE),)
    PYTHON_INCLUDE = $(shell python$(PYTHON_VERSION)-config --includes)
endif

ifeq ($(PYTHON_LDFLAGS),)
    PYTHON_LDFLAGS = $(shell python$(PYTHON_VERSION)-config --ldflags)
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
	$(CXX) -std=c++0x $(PYTHON_INCLUDE) $(BOOST_INCLUDE) $(JSON_INCLUDE) -I ../explore -fPIC -c pylibvw.cc -o pylibvw.o

clean:
	rm -f *.o $(PYLIBVW)

.PHONY: all clean things

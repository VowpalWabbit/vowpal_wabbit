#
# Critical variables are passed recursively (via the environment)
# by the top level Makefile when calling $(MAKE)
#
# If we don't know where to look for boost - it's a no go.
#
#ifeq ($(BOOST_LIBRARY),)
#  $(error Please run 'make' at the top level only)
#endif

PYTHON_VERSION = 2.7
PYTHON_INCLUDE = $(shell python$(PYTHON_VERSION)-config --includes)
PYTHON_LDFLAGS = $(shell python$(PYTHON_VERSION)-config --ldflags)

PYTHON_LIBS = -l boost_python
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
  PYTHON_LIBS = -l boost_python-mt
  PYLIBVW = pylibvw.dll
endif
ifneq ($(UNAME), Darwin)
  EXPORT_DYNAMIC = -Wl,--export-dynamic
endif

STDLIBS = $(BOOST_LIBRARY) $(LIBS) $(PYTHON_LIBS)

$(PYLIBVW): pylibvw.o  ../vowpalwabbit/libvw.a
	$(CXX) -shared $(EXPORT_DYNAMIC) pylibvw.o $(BOOST_LIBRARY) $(PYTHON_LDFLAGS) $(VWLIBS) $(STDLIBS) -o $(PYLIBVW)

pylibvw.o: pylibvw.cc
	$(CXX) -std=c++0x $(PYTHON_INCLUDE) $(BOOST_INCLUDE) -fPIC -c pylibvw.cc -o pylibvw.o

clean:
	rm -f *.o $(PYLIBVW)

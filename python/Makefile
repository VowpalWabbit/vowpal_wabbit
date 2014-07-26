#
# Critical variables are passed recursively (via the environment)
# by the top level Makefile when calling $(MAKE)
#
# If we don't know where to look for boost - it's a no go.
#
ifeq ($(BOOST_LIBRARY),)
  $(error Please run 'make' at the top level only)
endif

PYTHON_VERSION = 2.7
PYTHON_INCLUDE = /usr/include/python$(PYTHON_VERSION)

PYTHON_LIBS = 
VWLIBS = -L ../vowpalwabbit -l vw -l allreduce
STDLIBS = $(BOOST_LIBRARY) $(LIBS) $(PYTHONLIBS)

all: pylibvw.so

pylibvw.so: pylibvw.o
	$(CXX) -shared -Wl,--export-dynamic pylibvw.o $(BOOST_LIBRARY) -lboost_python -L/usr/lib/python$(PYTHON_VERSION)/config -lpython$(PYTHON_VERSION) $(VWLIBS) $(STDLIBS) -o pylibvw.so
 
pylibvw.o: pylibvw.cc
	$(CXX) -I$(PYTHON_INCLUDE) $(BOOST_INCLUDE) -fPIC -c pylibvw.cc -o pylibvw.o

clean:
	rm -f *.o *.so

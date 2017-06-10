BINARIES = vw active_interactor
MANPAGES = vw.1

VWLIBS := -L. -l vw -l allreduce
STDLIBS = $(BOOST_LIBRARY) $(LIBS)

all:
	cd ..; $(MAKE)

test:
	cd ..; $(MAKE) test

things:	config.h $(BINARIES)

%.1:	%
	help2man --no-info --name="Vowpal Wabbit -- fast online learning tool" ./$< > $@

config.h: ../configure.ac
	echo \#define PACKAGE_VERSION \"`grep AC_INIT ../configure.ac | cut -d '[' -f 3 | cut -d ']' -f 1`\" > config.h

vw_SOURCES = $(shell grep libvw_la_SOURCES Makefile.am | cut -d '=' -f 2)
vw_OBJECTS = $(patsubst %.cc,%.o,$(vw_SOURCES))
vw_DEPS = $(patsubst %.cc,%.d,$(vw_SOURCES))

allreduce_SOURCES = $(shell grep liballreduce_la_SOURCES Makefile.am | cut -d '=' -f 2)
allreduce_OBJECTS = $(patsubst %.cc,%.o,$(allreduce_SOURCES))

%.d:	%.cc config.h
	$(CXX) -std=c++0x -I ../rapidjson/include -MM $< > $@
-include $(vw_DEPS)

%.o:	 %.cc  %.h
	$(CXX) $(FLAGS) -c $< -o $@

%.o:	 %.cc
	$(CXX) $(FLAGS) -c $< -o $@

liballreduce.a: $(allreduce_OBJECTS)
	ar rcs $@ $+

libvw.a: $(filter-out vw.o,$(vw_OBJECTS))
	ar rcs $@ $+

vw: main.o libvw.a liballreduce.a
	$(CXX) $(FLAGS) -o $@ $< $(VWLIBS) $(STDLIBS)

active_interactor: active_interactor.cc
	$(CXX) $(FLAGS) -o $@ $+

install: $(BINARIES)
	cp $(BINARIES) /usr/local/bin; cd cluster; $(MAKE) install; cd java; $(MAKE) install

clean:
	rm -f  *.o *.d $(BINARIES) *~ $(MANPAGES) libvw.a liballreduce.a

python: vw
	cd .. ; $(MAKE) python

.PHONY: all clean install test things

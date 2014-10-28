BINARIES = vw active_interactor
SHARED_LIBRARIES = libvw.so liballreduce.so libvw_c_wrapper.so
MANPAGES = vw.1

VWLIBS := -Wl,-rpath,. -L. -l vw -l allreduce
STDLIBS = $(BOOST_LIBRARY) $(LIBS)

all:
	cd ..; $(MAKE)

test:
	cd ..; $(MAKE) test

things:	config.h $(BINARIES) $(SHARED_LIBRARIES)

%.1:	%
	help2man --no-info --name="Vowpal Wabbit -- fast online learning tool" ./$< > $@

config.h: ../configure.ac
	echo \#define PACKAGE_VERSION \"`grep AC_INIT ../configure.ac | cut -d '[' -f 3 | cut -d ']' -f 1`\" > config.h

vw_SOURCES = $(shell grep libvw_la_SOURCES Makefile.am | cut -d '=' -f 2)
vw_OBJECTS = $(patsubst %.cc,%.o,$(vw_SOURCES))
vw_DEPS = $(patsubst %.cc,%.d,$(vw_SOURCES))

%.d:	%.cc config.h
	$(CXX) -MM $< > $@

-include $(vw_DEPS)

%.o:	 %.cc  %.h
	$(CXX) $(FLAGS) -c $< -o $@

%.o:	 %.cc
	$(CXX) $(FLAGS) -c $< -o $@

liballreduce.a:	allreduce.o
	ar rcs $@ $<

ifneq ($(UNAME), Darwin)
  EXPORT_DYNAMIC = -Wl,--export-dynamic
endif

liballreduce.so: allreduce.o
	$(CXX) $(FLAGS) -shared $(EXPORT_DYNAMIC) -o $@ $<

libvw.a: $(filter-out vw.o,$(vw_OBJECTS))
	ar rcs $@ $+

libvw.so: $(filter-out vw.o,$(vw_OBJECTS)) liballreduce.so
	$(CXX) $(FLAGS) -shared $(EXPORT_DYNAMIC) -o $@ $(filter-out liballreduce.so,$+) -L. -l allreduce $(STDLIBS)

vw: main.o libvw.a liballreduce.a libvw.so liballreduce.so
	$(CXX) $(FLAGS) -o $@ $< $(VWLIBS) $(STDLIBS)

active_interactor: active_interactor.cc
	$(CXX) $(FLAGS) -o $@ $+

libvw_c_wrapper.so : vwdll.o libvw.so
	$(CXX) $(FLAGS) -shared $(EXPORT_DYNAMIC) -o $@ $< $(VWLIBS)

install: $(BINARIES)
	cp $(BINARIES) /usr/local/bin; cd cluster; $(MAKE) install

clean:
	rm -f  *.o *.d $(BINARIES) *~ $(MANPAGES) *.a *.so

.PHONY: all clean install test things

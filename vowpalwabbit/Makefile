BINARIES = vw active_interactor
MANPAGES = vw.1

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

%.d:	%.cc config.h
	$(CXX) -MM $< > $@

-include $(vw_DEPS)

%.o:	 %.cc  %.h
	$(CXX) $(FLAGS) -c $< -o $@

%.o:	 %.cc
	$(CXX) $(FLAGS) -c $< -o $@

liballreduce.a:	allreduce.o
	ar rcs $@ $<

libvw.a: $(filter-out vw.o,$(vw_OBJECTS))
	ar rcs $@ $+

vw: main.o libvw.a liballreduce.a
	$(CXX) $(FLAGS) -L$(BOOST_LIBRARY) -L. -o $@ $< -l vw -l allreduce $(LIBS)

active_interactor: active_interactor.cc
	$(CXX) $(FLAGS) -o $@ $+

install: $(BINARIES)
	cp $(BINARIES) /usr/local/bin; cd cluster; $(MAKE) install

clean:
	rm -f  *.o *.d $(BINARIES) *~ $(MANPAGES) libvw.a

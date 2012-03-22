BINARIES = vw active_interactor
MANPAGES = vw.1

all:	config.h depend $(BINARIES)

%.1:	%
	help2man --no-info --name="Vowpal Wabbit -- fast online learning tool" ./$< > $@

config.h: 
	echo \#define PACKAGE_VERSION \"`grep AC_INIT ../configure.ac | cut -d '[' -f 3 | cut -d ']' -f 1`\" > config.h

vw_SOURCES = $(shell grep vw_SOURCES Makefile.am | cut -d '=' -f 2)

depend:	$(vw_SOURCES) config.h
	gcc -MM $(vw_SOURCES) > depend

-include depend

%.o:	 %.cc  %.h
	$(COMPILER) $(FLAGS) -c $< -o $@

%.o:	 %.cc
	$(COMPILER) $(FLAGS) -c $< -o $@

liballreduce.a:	allreduce.o
	ar rcs $@ $<

vw_OBJECTS = $(patsubst %.cc,%.o,$(vw_SOURCES))

vw: $(vw_OBJECTS) liballreduce.a
	$(COMPILER) $(FLAGS) -L$(BOOST_LIBRARY) -L. -o $@ $+ $(LIBS)

active_interactor: active_interactor.cc
	$(COMPILER) $(FLAGS) -o $@ $+

install: $(BINARIES)
	cp $(BINARIES) /usr/local/bin; cd cluster; $(MAKE) install

clean:
	rm -f  *.o $(BINARIES) *~ $(MANPAGES)

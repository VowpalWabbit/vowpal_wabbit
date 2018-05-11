
vwslim_SOURCES = $(shell grep libvwslim_la_SOURCES Makefile.am | cut -d '=' -f 2)
vwslim_OBJECTS = $(patsubst %.cc,%.o,$(vwslim_SOURCES))

vwslimtest_SOURCES = ut_opts.cpp ut_vw.cpp vw_util.cpp ut_main.cc 
vwslimtest_OBJECTS = $(patsubst %.cc,%.o,$(vwslimtest_SOURCES))

SLIMFLAGS = $(FLAGS) -D VW_NOEXCEPT -fno-exceptions -I ../vowpalwabbit

all: 
	cd ..; $(MAKE) vwslim

things: libvwslim.a ut_main

# generate test data
data.h:
	cd data ; PATH=$$PATH:../../vowpalwabbit ./generate-data.sh

ut_main: $(vwslimtest_OBJECTS) data.h gtest-all.o gmock-all.o libvwslim.a 
	$(CXX) $(SLIMFLAGS) -o $@ $+ -l pthread

test: 
	./ut_main

%.d:	%.cc config.h
	$(CXX) $(SLIMFLAGS) -MM $< > $@
-include $(vw_DEPS)

# build google test framework
gtest-all.o: /usr/src/gtest/src/gtest-all.cc
	$(CXX) $(SLIMFLAGS) -I /usr/src/gtest -c $< -o $@

# build google mock framework
gmock-all.o: /usr/src/gmock/src/gmock-all.cc
	$(CXX) $(SLIMFLAGS) -I /usr/src/gmock -c $< -o $@

%.o:	 %.cc  %.h
	$(CXX) $(SLIMFLAGS) -c $< -o $@


%.o:	 %.cc
	$(CXX) $(SLIMFLAGS) -c $< -o $@

libvwslim.a: $(vwslim_OBJECTS)
	ar rcs $@ $+

clean:
	rm -f  *.o *.d *~ libvwslim.a ut_main

.PHONY: all things test

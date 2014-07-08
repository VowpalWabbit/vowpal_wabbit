# -- default standard locations
UNAME := $(shell uname)

VWLIBS = -L ../vowpalwabbit -l vw -l allreduce

BOOST_INCLUDE = -I /usr/include -I /usr/local/include
BOOST_LIBRARY = -L /usr/lib -L /usr/local/lib

BOOST_PROGRAM_OPTIONS = $(BOOST_LIBRARY) -l boost_program_options
STDLIBS = $(BOOST_PROGRAM_OPTIONS) -l pthread -l z

IFLAGS=$(BOOST_INCLUDE)

# -- envs needing overrides
ifeq ($(UNAME), FreeBSD)
  STDLIBS = $(BOOST_PROGRAM_OPTIONS) -l pthread -l z -l compat
endif
ifeq "CYGWIN" "$(findstring CYGWIN,$(UNAME))"
  BOOST_PROGRAM_OPTIONS = $(BOOST_LIBRARY) -l boost_program_options-mt
  STDLIBS = $(BOOST_PROGRAM_OPTIONS) -l pthread -l z
endif
ifeq ($(UNAME), Darwin)
  # On Macs, the location isn't always clear
  #	brew uses /usr/local
  #	but /opt/local seems to be preferred by some users
  #	so we try them both
  BOOST_INCLUDE = $(BOOST_INCLUDE) -I /opt/local/include
  BOOST_LIBRARY = $(BOOST_LIBRARY) -L /opt/local/lib
  BOOST_PROGRAM_OPTIONS = $(BOOST_LIBRARY) -l boost_program_options-mt
  STDLIBS = $(BOOST_PROGRAM_OPTIONS) -lboost_serialization-mt -l pthread -l z
endif

all: ezexample_predict ezexample_train library_example recommend gd_mf_weights

ezexample_predict: ezexample_predict.cc ../vowpalwabbit/libvw.a ezexample.h
	$(CXX) -g $(IFLAGS) -o $@ $< $(VWLIBS) $(STDLIBS)

ezexample_predict_threaded: ezexample_predict_threaded.cc ../vowpalwabbit/libvw.a ezexample.h
	$(CXX) -g $(IFLAGS)  -o $@ $< $(VWLIBS) $(BOOST_PROGRAM_OPTIONS) -l z -l boost_thread

ezexample_train: ezexample_train.cc ../vowpalwabbit/libvw.a ezexample.h
	$(CXX) -g $(IFLAGS) -o $@ $< $(VWLIBS) $(STDLIBS)

library_example: library_example.cc ../vowpalwabbit/libvw.a
	$(CXX) -g $(IFLAGS) -o $@ $< $(VWLIBS) $(STDLIBS)

recommend: recommend.cc ../vowpalwabbit/libvw.a ezexample.h
	$(CXX) -g $(IFLAGS) -o $@ $< $(VWLIBS) $(STDLIBS)

gd_mf_weights: gd_mf_weights.cc ../vowpalwabbit/libvw.a
	$(CXX) -g $(IFLAGS) -o $@ $< $(VWLIBS) $(STDLIBS)

clean:
	rm -f *.o ezexample_predict ezexample_train library_example recommend ezexample_predict_threaded

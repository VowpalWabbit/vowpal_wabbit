BOOST_PROGRAM_OPTIONS = -l boost_program_options
ifeq ($(UNAME), FreeBSD)
  BOOST_PROGRAM_OPTIONS = $(BOOST_LIBRARY) -l boost_program_options
endif
ifeq "CYGWIN" "$(findstring CYGWIN,$(UNAME))"
  BOOST_PROGRAM_OPTIONS = $(BOOST_LIBRARY) -l boost_program_options-mt
endif
ifeq ($(UNAME), Darwin)
  BOOST_PROGRAM_OPTIONS = $(BOOST_LIBRARY) -l boost_program_options-mt
endif

# $(BOOST_LIBRARY) is set from top level Makefile to -L /where/boost/is
VWLIBS := -L ../vowpalwabbit -l vw -l allreduce
STDLIBS = $(BOOST_PROGRAM_OPTIONS) -l z -l pthread

all: ezexample_predict ezexample_train library_example recommend gd_mf_weights

ezexample_predict: ezexample_predict.cc ../vowpalwabbit/libvw.a ezexample.h
	$(CXX) -g $(FLAGS) -o $@ $< $(VWLIBS) $(STDLIBS)

ezexample_predict_threaded: ezexample_predict_threaded.cc ../vowpalwabbit/libvw.a ezexample.h
	$(CXX) -g $(FLAGS)  -o $@ $< $(VWLIBS) $(BOOST_PROGRAM_OPTIONS) -l z -l boost_thread

ezexample_train: ezexample_train.cc ../vowpalwabbit/libvw.a ezexample.h
	$(CXX) -g $(FLAGS) -o $@ $< $(VWLIBS) $(STDLIBS)

library_example: library_example.cc ../vowpalwabbit/libvw.a
	$(CXX) -g $(FLAGS) -o $@ $< $(VWLIBS) $(STDLIBS)

recommend: recommend.cc ../vowpalwabbit/libvw.a ezexample.h
	$(CXX) -g $(FLAGS) -o $@ $< $(VWLIBS) $(STDLIBS)

gd_mf_weights: gd_mf_weights.cc ../vowpalwabbit/libvw.a
	$(CXX) -g $(FLAGS) -o $@ $< $(VWLIBS) $(STDLIBS)

clean:
	rm -f *.o ezexample_predict ezexample_train library_example recommend ezexample_predict_threaded

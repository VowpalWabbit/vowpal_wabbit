#
# Critical variables are passed recursively (via the environment)
# by the top level Makefile when calling $(MAKE)
#
# If we don't know where to look for boost - it's a no go.
#
#ifeq ($(BOOST_LIBRARY),)
#  $(error Please run 'make' at the top level only)
#endif

VWLIBS = -L ../vowpalwabbit -l vw -l allreduce
STDLIBS = $(BOOST_LIBRARY) $(LIBS)

all:
	cd ..; $(MAKE) library_example

things: ezexample_predict ezexample_train library_example recommend gd_mf_weights test_search # ezexample_predict_threaded

ezexample_predict: ezexample_predict.cc ../vowpalwabbit/libvw.a
	$(CXX) -g $(FLAGS) -o $@ $< $(VWLIBS) $(STDLIBS)

ezexample_predict_threaded: ezexample_predict_threaded.cc ../vowpalwabbit/libvw.a
	$(CXX) -g $(FLAGS)  -o $@ $< $(VWLIBS) $(BOOST_PROGRAM_OPTIONS) -l z -l boost_thread

ezexample_train: ezexample_train.cc ../vowpalwabbit/libvw.a
	$(CXX) -g $(FLAGS) -o $@ $< $(VWLIBS) $(STDLIBS)

library_example: library_example.cc ../vowpalwabbit/libvw.a
	$(CXX) -g $(FLAGS) -o $@ $< $(VWLIBS) $(STDLIBS)

test_search: test_search.cc ../vowpalwabbit/libvw.a
	$(CXX) -g $(FLAGS) -o $@ $< $(VWLIBS) $(STDLIBS)

recommend: recommend.cc ../vowpalwabbit/libvw.a
	$(CXX) -g $(FLAGS) -o $@ $< $(VWLIBS) $(STDLIBS)

gd_mf_weights: gd_mf_weights.cc ../vowpalwabbit/libvw.a
	$(CXX) -g $(FLAGS) -o $@ $< $(VWLIBS) $(STDLIBS)

clean:
	rm -f *.o ezexample_predict ezexample_train library_example test_search recommend ezexample_predict_threaded


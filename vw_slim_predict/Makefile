
BINARIES = vw_slim_predict

all:
	cd ..; $(MAKE) vw_slim_predict

things: $(BINARIES)

vw_slim_predict_SOURCES = $(shell grep vw_slim_predict_SOURCES Makefile.am | cut -d '=' -f 2)
vw_slim_predict_OBJECTS = $(patsubst %.cc,%.o,$(vw_slim_predict_SOURCES))

%.d:	%.cc
	$(CXX) -std=c++0x -I ../vowpalwabbit -MM $< > $@
-include $(vw_DEPS)

%.o:	 %.cc  %.h
	$(CXX) $(FLAGS) -I ../vowpalwabbit -c $< -o $@

%.o:	 %.cc
	$(CXX) $(FLAGS) -I ../vowpalwabbit -c $< -o $@

vw_slim_predict: $(vw_slim_predict_OBJECTS)
	$(CXX) $(FLAGS) -I ../vowpalwabbit -o $@ $+ $(STDLIBS)

clean:
	rm -f  *.o *.d $(BINARIES) *~ 

.PHONY: all clean test things

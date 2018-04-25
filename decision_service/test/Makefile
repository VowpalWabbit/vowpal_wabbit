TARGETS = ds_concurrent_queue_test ds_event_test
CXX = g++
CXXFLAGS = -std=c++11

all: $(TARGETS)

test: $(TARGETS) $(addprefix run-,$(TARGETS))

%: %.cc
	$(CXX) $(CXXFLAGS) -o $@ $^ -lboost_unit_test_framework -DBOOST_TEST_DYN_LINK

run-%: %
	-./$^ --output_format=XML --log_level=test_suite > $(^)-report.xml

clean:
	rm -f $(TARGETS)
	rm -f *.o
	rm -f *-report.xml

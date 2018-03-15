TARGETS = ds_driver_test ds_configuration_test ds_logger_test ds_serializer_test ds_predictor_test ds_ranking_response_test

all: $(TARGETS)

test: $(TARGETS) $(addprefix run-,$(TARGETS))

%: %.cc
	$(CXX) -o$@ $^ -lboost_unit_test_framework -DBOOST_TEST_DYN_LINK

run-%: %
	-./$^ --output_format=XML --log_level=test_suite > $(^)-report.xml

clean:
	rm $(TARGETS) *-report.xml

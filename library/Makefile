ifeq ($(UNAME), FreeBSD)
BOOST_PROGRAM_OPTIONS = boost_program_options
endif
ifeq "CYGWIN" "$(findstring CYGWIN,$(UNAME))"
BOOST_PROGRAM_OPTIONS = boost_program_options-mt
endif
ifeq ($(UNAME), Darwin)
BOOST_PROGRAM_OPTIONS = boost_program_options-mt
endif

all: ezexample_predict ezexample_train library_example

ezexample_predict: ezexample_predict.cc ../vowpalwabbit/libvw.a ezexample.h
	g++ -g -o $@ $< -L ../vowpalwabbit -l vw -l allreduce -l $(BOOST_PROGRAM_OPTIONS) -l z -l pthread

ezexample_train: ezexample_train.cc ../vowpalwabbit/libvw.a ezexample.h
	g++ -g -o $@ $< -L ../vowpalwabbit -l vw -l allreduce -l $(BOOST_PROGRAM_OPTIONS) -l z -l pthread

library_example: library_example.cc ../vowpalwabbit/libvw.a
	g++ -g -o $@ $< -L ../vowpalwabbit -l vw -l allreduce -l $(BOOST_PROGRAM_OPTIONS) -l z -l pthread

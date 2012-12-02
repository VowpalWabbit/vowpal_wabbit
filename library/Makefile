ezexample_predict: ezexample_predict.cc ../vowpalwabbit/libvw.a ezexample.h
	g++ -g -o $@ $< -L ../vowpalwabbit -l vw -l allreduce -l boost_program_options -l z -l pthread

ezexample_train: ezexample_train.cc ../vowpalwabbit/libvw.a ezexample.h
	g++ -g -o $@ $< -L ../vowpalwabbit -l vw -l allreduce -l boost_program_options -l z -l pthread

library_example: library_example.cc ../vowpalwabbit/libvw.a
	g++ -g -o $@ $< -L ../vowpalwabbit -l vw -l allreduce -l boost_program_options -l z -l pthread

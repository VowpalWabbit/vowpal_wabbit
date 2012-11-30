ezexample_predict: ezexample_predict.cc ../vowpalwabbit/libvw.a ezexample.h
	g++ -g -o $@ -l boost_program_options -l z -l pthread $< -L ../vowpalwabbit -l vw -l allreduce

ezexample_train: ezexample_train.cc ../vowpalwabbit/libvw.a ezexample.h
	g++ -g -o $@ -l boost_program_options -l z -l pthread $< -L ../vowpalwabbit -l vw -l allreduce

library_example: library_example.cc ../vowpalwabbit/libvw.a
	g++ -g -o $@ $< -L ../vowpalwabbit -l vw -l allreduce -l boost_program_options -l z -l pthread

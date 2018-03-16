exec python repeat.py 1024 ../vowpalwabbit/vw train-sets/rcv1_small.dat --holdout_after 100 --quiet
#Two race conditions to worry about
#
#Scencario 1: done-then-dispatch
#1. Parsing thread sets p->done
#2. learning thread get_example notes end_parsed_examples = used_index
#3. learning thread early terminates.
#4. Parsing thread calls dispatch_example.
#
#Scenario 2: dispatch-then-done
#1. parser thread dispatches example
#2. learner thread consumes examples
#3. learner thread hangs on p->example_available
#4. parser thread sets done to true.
#
#Fix: Use scenario 2 but have done raise the examples_available flag to unblock the learning thread.

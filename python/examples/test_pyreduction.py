import sys, os
import math
from vowpalwabbit import pyvw

class NoopPythonReduction(pyvw.Copperhead):
    def _predict(self, ec, learner):
        #print("hello there I'm predicting stuff")
        learner.predict(ec)

    def _learn(self, ec, learner):
        #print("hello there I can also learn stuff btw the total_sum_feat_sq is " + str(ec.get_total_sum_feat_sq()))
        learner.learn(ec)

# this is a recreation of the impl in vw
# see https://github.com/VowpalWabbit/vowpal_wabbit/blob/ac3a2c21a9760b68ce49368b11a35bf95faeb8b8/vowpalwabbit/binary.cc
class BinaryPythonReduction(pyvw.Copperhead):
    def _predict(self, ec, learner):
        # print("hello there I'm predicting stuff")
        learner.predict(ec)

    def _learn(self, ec, learner):
        learner.learn(ec)

        if ec.get_simplelabel_prediction() > 0:
            ec.set_simplelabel_prediction(1)
        else:
            ec.set_simplelabel_prediction(-1)

        temp_label = ec.get_simplelabel_label()

        # going to have to expose FLT_MAX?
        if temp_label != sys.float_info.max:
            if math.fabs(temp_label) != 1.0:
                print("You are using label " + temp_label + " not -1 or 1 as loss function expects!")
            elif temp_label == ec.get_scalar():
                ec.set_loss(0)
            else:
                ec.set_loss(ec.get_simplelabel_weight())

print(os.getpid())

# this should match cpp_binary() output
# doesn't do anything, runs in python see class impl NoopPythonicReductions
def noop_example():
    vw = pyvw.vw(arg_str="--loss_function logistic --binary --active_cover --oracular -d test/train-sets/rcv1_small.dat", partial_initialize = True)
    
    reduction_stack = []
    reduction = vw.pop_reduction()
    while reduction is not None:
        print("popping reduction")
        reduction_stack.append(reduction);
        reduction = vw.pop_reduction()

    # noop reduction runs just before GD
    vw.push_reduction(reduction_stack.pop())
    vw.create_and_push_custom_reduction("my_noop", NoopPythonReduction)
    while len(reduction_stack) > 0:
        print("pushing reduction")
        vw.push_reduction(reduction_stack.pop())

    vw.complete_initialize()
    vw.run_parser()
    vw.finish()

# this should match cpp_binary() output
# mirror implementation of the cpp, runs in python see class impl BinaryPythonReductions
def python_binary():
    vw = pyvw.vw(arg_str="--loss_function logistic --active_cover --oracular -d test/train-sets/rcv1_small.dat")
    vw.run_parser()
    vw.finish()

# this should be the baseline
def cpp_binary():
    vw = pyvw.vw("--loss_function logistic --binary --active_cover --oracular -d test/train-sets/rcv1_small.dat")
    vw.run_parser()
    vw.finish()

#print("python")
#python_binary()
print("noop")
noop_example()
print("cpp")
cpp_binary()

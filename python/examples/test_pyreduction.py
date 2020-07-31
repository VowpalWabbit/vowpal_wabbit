from __future__ import print_function

import sys, os
import math
from vowpalwabbit import pyvw

class NoopPythonicReduction(pyvw.Copperhead):
    def _predict(self):
        print("hello there I'm predicting stuff")

    def _learn(self, ec, learner):
        # print("hello there I can also learn stuff btw the total_sum_feat_sq is " + str(ec.get_total_sum_feat_sq()))
        learner.learn(ec)

class BinaryPythonReduction(pyvw.Copperhead):
    def _predict(self):
        print("hello there I'm predicting stuff")

    def _learn(self, ec, learner):
        learner.learn(ec)

        if ec.get_simplelabel_prediction() > 0:
            ec.set_simplelabel_prediction(1)
        else:
            ec.set_simplelabel_prediction(-1)

        temp_label = ec.get_simplelabel_label()
        some_scalar = ec.get_scalar()

        # going to have to expose FLT_MAX?
        if temp_label != sys.float_info.max:
            if math.fabs(temp_label) != 1.0:
                print("You are using label " + temp_label << " not -1 or 1 as loss function expects!")
            elif temp_label == some_scalar:
                ec.set_loss(0)
            else:
                ec.set_loss(ec.get_simplelabel_weight())

print(os.getpid())

# this should match cpp_binary() output
# doesn't do anything, runs in python see class impl NoopPythonicReductions
def noop_example():
    vw = pyvw.createWithCustomPythonReduction(NoopPythonicReduction, "--loss_function logistic --binary --active_cover --oracular -d /root/vw/test/train-sets/rcv1_small.dat")
    vw.run_parser()
    vw.finish()

# this should match cpp_binary() output
# mirror implementation of the cpp, runs in python see class impl BinaryPythonReductions
def python_binary():
    vw = pyvw.createWithCustomPythonReduction(BinaryPythonReduction, "--loss_function logistic --active_cover --oracular -d /root/vw/test/train-sets/rcv1_small.dat")
    vw.run_parser()
    vw.finish()

# this should be the baseline
def cpp_binary():
    vw = pyvw.vw("--loss_function logistic --binary --active_cover --oracular -d /root/vw/test/train-sets/rcv1_small.dat")
    vw.run_parser()
    vw.finish()

print("noop")
noop_example()
print("python")
python_binary()
print("cpp")
cpp_binary()
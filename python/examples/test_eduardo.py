from __future__ import print_function

import sys, os
from vowpalwabbit import pyvw

class MyCustomPythonicReduction(pyvw.Copperhead):
    def _predict(self):
        print("hello there I'm predicting stuff")

    def _learn(self, ec):
        print("hello there I can also learn stuff btw the total_sum_feat_sq is " + str(ec.get_total_sum_feat_sq()))

        self._call_base_learn(ec)

print(os.getpid())

# initialize VW as usual, but use 'hook' as the search_task
# vw = pyvw.vw("--red_python --cb_adf --dsjson -c -d /root/vw/python/examples/prueba.json --cb_type ips -l 1 --power_t 0 -P 1 ")
vw = pyvw.vw("--red_python --loss_function logistic --binary --active_cover --oracular -d /root/vw/test/train-sets/rcv1_small.dat")
# vw = pyvw.vw("--red_python --loss_function logistic --active_cover --oracular -d /root/vw/test/train-sets/rcv1_small.dat")

customPythonReduction = vw.init_python_reduction_task(MyCustomPythonicReduction)
# customPythonReduction = vw.init_python_reduction_task(BinaryPythonReduction)

vw.run_parser()

# class BinaryPythonReduction(pyvw.Copperhead):
#     def _predict(self):
#         print("hello there I'm predicting stuff")

#     def _learn(self, ec):
#         # print("hello there I can also learn stuff btw the total_sum_feat_sq is " + str(ec.get_total_sum_feat_sq()))
#         self._call_base_learn(ec)

#         if ec.get_simplelabel_prediction() > 0:
#             ec.set_simplelabel_prediction(1)
#         else:
#             ec.set_simplelabel_prediction(-1)

#         if ec.ex_get_simplelabel_label() >= sys.float_info.max:
#             print("hola")
#             if ec.ex_get_simplelabel_label() != 1.0:
#                 print("you are using label blah")
#             # if ec.ex_get_simplelabel_label() != 1.f:
#             #     print("you are using label blah")
#             elif ec.ex_get_simplelabel_label == ec.get_simplelabel_prediction:
#                 ec.set_loss(0)
#             else:
#                 ec.set_loss(ec.ex_get_simplelabel_weight())



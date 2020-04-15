from __future__ import print_function

import sys, os
from vowpalwabbit import pyvw

class MyCustomPythonicReduction(pyvw.Copperhead):
    def _predict(self):
        print("hello there I'm predicting stuff")

    def _learn(self):
        print("hello there I can also learn stuff")


print(os.getpid())

# initialize VW as usual, but use 'hook' as the search_task
# vw = pyvw.vw("--red_python --cb_adf --dsjson -c -d /root/vw/python/examples/prueba.json --cb_type ips -l 1 --power_t 0 -P 1 ")
vw = pyvw.vw("--red_python --loss_function logistic --binary --active_cover --oracular -d /root/vw/test/train-sets/rcv1_small.dat")

customPythonReduction = vw.init_python_reduction_task(MyCustomPythonicReduction)

vw.run_parser()

# tell VW to construct your search task object
sequenceLabeler = vw.init_search_task(SequenceLabeler)

# train it on the above dataset ten times; the my_dataset.__iter__ feeds into _run above
# print('training!')
# i = 0
# while i < 10:
#     sequenceLabeler.learn(my_dataset)
#     i += 1

# now see the predictions on a test sentence
# print('predicting!', file=sys.stderr)
# print(sequenceLabeler.predict( [(1,w) for w in "the sandwich ate a monster".split()] ))
# print('should have printed: [1, 2, 3, 1, 2]')

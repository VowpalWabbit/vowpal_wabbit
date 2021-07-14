from __future__ import print_function

from vowpalwabbit import pyvw


class LatentVariableClassifier(pyvw.SearchTask):
    def __init__(self, vw, sch, num_actions):
        pyvw.SearchTask.__init__(self, vw, sch, num_actions)
        sch.set_options( sch.AUTO_CONDITION_FEATURES )

    def _run(self, y_x):
        y,(x0,x1) = y_x

        ex = self.example({'x': [('x0',x0), ('x1',x1)]})
        h  = self.sch.predict(examples=ex, my_tag=1, oracle=None) * 2 - 3

        ex = self.example({'x': [('x0',x0), ('x1',x1), ('x0h',x0*h)]})
        p  = self.sch.predict(examples=ex, my_tag=2, oracle=y, condition=(1,'h'))

        self.sch.loss( 0. if p == y else 1. )
        return p

my_dataset = [ (1, (-1, -1)),
               (1, (+1, +1)),
               (2, (-1, +1)),
               (2, (+1, -1)) ]


vw = pyvw.vw("--search 2 --search_task hook --ring_size 1024 --search_alpha 1e-2")
lv = vw.init_search_task(LatentVariableClassifier)

print('training')
for i in range(100):
    lv.learn(my_dataset)

print('testing')
for (y,x) in my_dataset:
    print('pred =', lv.predict( (0,x) ))

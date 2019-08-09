from __future__ import print_function

from vowpalwabbit import pyvw


def my_predict(vw, ex):
    pp = 0.
    for f,v in ex.iter_features():
        pp += vw.get_weight(f) * v
    return pp

def ensure_close(a,b,eps=1e-6):
    if abs(a-b) > eps:
        raise Exception("test failed: expected " + str(a) + " and " + str(b) + " to be " + str(eps) + "-close, but they differ by " + str(abs(a-b)))

###############################################################################
vw = pyvw.vw("--quiet")


###############################################################################
vw.learn("1 |x a b")


###############################################################################
print('# do some stuff with a read example:')
ex = vw.example("1 |x a b |y c")
ex.learn() ; ex.learn() ; ex.learn() ; ex.learn()

updated_pred = ex.get_updated_prediction()
print('current partial prediction =', updated_pred)

# compute our own prediction
print('        my view of example =', str([(f,v,vw.get_weight(f)) for f,v in ex.iter_features()]))
my_pred = my_predict(vw, ex)
print('     my partial prediction =', my_pred)
ensure_close(updated_pred, my_pred)
print('')
vw.finish_example(ex)

###############################################################################
print('# make our own example from scratch')
ex = vw.example()
ex.set_label_string("0")
ex.push_features('x', ['a', 'b'])
ex.push_features('y', [('c', 1.)])
ex.setup_example()

print('        my view of example =', str([(f,v,vw.get_weight(f)) for f,v in ex.iter_features()]))
my_pred2 = my_predict(vw, ex)
print('     my partial prediction =', my_pred2)
ensure_close(my_pred, my_pred2)

ex.learn() ; ex.learn() ; ex.learn() ; ex.learn()
print('  final partial prediction =', ex.get_updated_prediction())
ensure_close(ex.get_updated_prediction(), my_predict(vw,ex))
print('')
vw.finish_example(ex)


###############################################################################
exList = []
for i in range(120):    # note: if this is >=129, we hang!!!
    ex = vw.example()
    exList.append(ex)

# this is the safe way to delete the examples for VW to reuse:
for ex in exList:
    vw.finish_example(ex)

exList = [] # this should __del__ the examples, we hope :)
for i in range(120):    # note: if this is >=129, we hang!!!
    ex = vw.example()
    exList.append(ex)

for ex in exList:
    vw.finish_example(ex)


###############################################################################

for i in range(2):
    ex = vw.example("1 foo| a b")
    ex.learn()
    print('tag =', ex.get_tag())
    print('partial pred =', ex.get_partial_prediction())
    print('loss =', ex.get_loss())

    print('label =', ex.get_label())
    vw.finish_example(ex)



# to be safe, finish explicity (should happen by default anyway)
vw.finish()


###############################################################################
print('# test some save/load behavior')
vw = pyvw.vw("--quiet -f test.model")
ex = vw.example("1 |x a b |y c")
ex.learn() ; ex.learn() ; ex.learn() ; ex.learn()
before_save = ex.get_updated_prediction()
print('before saving, prediction =', before_save)
vw.finish_example(ex)

vw.finish()   # this should create the file

# now re-start vw by loading that model
vw = pyvw.vw("--quiet -i test.model")
ex = vw.example("1 |x a b |y c")  # test example
ex.learn()
after_save = ex.get_partial_prediction()
print(' after saving, prediction =', after_save)
vw.finish_example(ex)

ensure_close(before_save, after_save)
vw.finish()   # this should create the file

print('done!')

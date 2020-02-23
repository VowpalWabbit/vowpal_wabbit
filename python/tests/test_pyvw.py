import os

from vowpalwabbit import pyvw
from vowpalwabbit.pyvw import vw


BIT_SIZE = 18


class TestVW:

    model = vw(quiet=True, b=BIT_SIZE)

    def test_constructor(self):
        assert isinstance(self.model, vw)

    def test_learn_predict(self):
        ex = self.model.example('1 | a b c')
        init = self.model.predict(ex)
        assert init == 0
        self.model.learn(ex)
        assert self.model.predict(ex) > init
        assert self.model.predict({'x': "| a:1 b:0.5"}) == 0

    def test_get_tag(self):
        ex = self.model.example("1 foo| a b c")
        assert ex.get_tag() == 'foo'
        ex = self.model.example("1 1.0 bar| a b c")
        assert ex.get_tag() == 'bar'
        ex = self.model.example("1 'baz | a b c")
        assert ex.get_tag() == 'baz'

    def test_num_weights(self):
        assert self.model.num_weights() == 2 ** BIT_SIZE

    def test_get_weight(self):
        assert self.model.get_weight(0, 0) == 0

    def test_finish(self):
        assert not self.model.finished
        self.model.finish()
        assert self.model.finished


def test_delete():
    model = vw(quiet=True, b=BIT_SIZE)
    assert 'model' in locals()
    del model
    assert 'model' not in locals()


# Test prediction types

def test_scalar_prediction_type():
    model = vw(quiet=True)
    model.learn('1 | a b c')
    assert model.get_prediction_type() == model.pSCALAR
    prediction = model.predict(' | a b c')
    assert isinstance(prediction, float)
    del model


def test_scalars_prediction_type():
    n = 3
    model = vw(loss_function='logistic', oaa=n, probabilities=True, quiet=True)
    model.learn('1 | a b c')
    assert model.get_prediction_type() == model.pSCALARS
    prediction = model.predict(' | a b c')
    assert isinstance(prediction, list)
    assert len(prediction) == n
    del model


def test_multiclass_prediction_type():
    n = 3
    model = vw(loss_function='logistic', oaa=n, quiet=True)
    model.learn('1 | a b c')
    assert model.get_prediction_type() == model.pMULTICLASS
    prediction = model.predict(' | a b c')
    assert isinstance(prediction, int)
    del model


def test_prob_prediction_type():
    model = vw(loss_function='logistic', csoaa_ldf='mc', probabilities=True, quiet=True)
    multi_ex = [model.example('1:0.2 | a b c'), model.example('2:0.8  | a b c')]
    model.learn(multi_ex)
    assert model.get_prediction_type() == model.pPROB
    multi_ex = [model.example('1 | a b c'), model.example('2 | a b c')]
    prediction = model.predict(multi_ex)
    assert isinstance(prediction, float)
    del model


def test_action_scores_prediction_type():
    model = vw(loss_function='logistic', csoaa_ldf='m', quiet=True)
    multi_ex = [model.example('1:1 | a b c'), model.example('2:-1  | a b c')]
    model.learn(multi_ex)
    assert model.get_prediction_type() == model.pMULTICLASS
    multi_ex = [model.example('1 | a b c'), model.example('2 | a b c')]
    prediction = model.predict(multi_ex)
    assert isinstance(prediction, int)
    del model


def test_action_probs_prediction_type():
    model = vw(cb_explore=2, ngram=2, quiet=True)
    model.learn('1 | a b c')
    assert model.get_prediction_type() == model.pACTION_PROBS
    prediction = model.predict(' | a b c')
    assert isinstance(prediction, list)
    del model


def test_multilabel_prediction_type():
    model = vw(multilabel_oaa=4, quiet=True)
    model.learn('1 | a b c')
    assert model.get_prediction_type() == model.pMULTILABELS
    prediction = model.predict(' | a b c')
    assert isinstance(prediction, list)
    del model


def test_cbandits_label():
    model = vw(cb=4, quiet=True)
    cbl = pyvw.cbandits_label(model.example('1 |'))
    assert cbl.costs[0].label == 1
    assert cbl.prediction == 0
    assert str(cbl) == '1:3.4028234663852886e+38'
    del model


def test_cost_sensitive_label():
    model = vw(csoaa=4, quiet=True)
    csl = pyvw.cost_sensitive_label(model.example('1 |'))
    assert csl.costs[0].label == 1
    assert csl.prediction == 0
    assert str(csl) == '1:3.4028234663852886e+38'
    del model


def test_regressor_args():
    # load and parse external data file
    data_file = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'resources', 'train.dat')
    model = vw(oaa=3, data=data_file, passes=30, c=True, k=True)
    assert model.predict('| feature1:2.5') == 1

    # update model in memory
    for _ in range(10):
        model.learn('3 | feature1:2.5')
    assert model.predict('| feature1:2.5') == 3

    # save model
    model.save('tmp.model')
    del model

    # load initial regressor and confirm updated prediction
    new_model = vw(i='tmp.model', quiet=True)
    assert new_model.predict('| feature1:2.5') == 3
    del new_model

    # clean up
    os.remove('{}.cache'.format(data_file))
    os.remove('tmp.model')


def test_keys_with_list_of_values():
    # No exception in creating and executing model with a key/list pair
    model = vw(quiet=True, q=["fa", "fb"])
    model.learn('1 | a b c')
    prediction = model.predict(' | a b c')
    assert isinstance(prediction, float)
    del model

def test_parse():
    model = vw(quiet=True, cb_adf=True)
    ex = model.parse("| a:1 b:0.5\n0:0.1:0.75 | a:0.5 b:1 c:2")
    assert len(ex) == 2

    ex = model.parse("""| a:1 b:0.5
    0:0.1:0.75 | a:0.5 b:1 c:2""")
    assert len(ex) == 2

    ex = model.parse("""
    | a:1 b:0.5
    0:0.1:0.75 | a:0.5 b:1 c:2
    """)
    assert len(ex) == 2

    ex = model.parse(["| a:1 b:0.5", "0:0.1:0.75 | a:0.5 b:1 c:2"])
    assert len(ex) == 2

    # To test the prediction on multiline examples
    finish = model.finish_example(ex)
    assert model.predict(ex) == [0.0, 0.0]
    del model

def test_numspace_id():
    vw_ex = vw(quiet=True)
    ex = vw_ex.example('1 |a two features |b more features here')
    nm1 = pyvw.namespace_id(ex, 2)
    assert nm1.id == 2
    assert nm1.ord_ns == 128
    assert nm1.ns == '\x80'
    nm2 = pyvw.namespace_id(ex, '')
    assert nm2.id is None
    assert nm2.ord_ns == 32
    assert nm2.ns == ' '

def test_example_namespace():
    vw_ex = vw(quiet=True)
    ex = vw_ex.example('1 |a two features |b more features here')
    nmid = pyvw.namespace_id(ex, 1)
    exm = pyvw.example_namespace(ex, nmid)
    assert isinstance(exm.ex, pyvw.example)
    assert isinstance(exm.ns, pyvw.namespace_id)
    assert exm.ns_hash is None
    assert exm.num_features_in() == 3
    assert exm[2] == (11617, 1.0)
    assert exm.iter_features()
    assert exm.pop_feature()
    exm.push_features(nmid, ['c', 'd'])
    assert exm.num_features_in() == 4

def test_simple_label():
    vw_ex = vw(quiet=True)
    ex = vw_ex.example('1 |a two features |b more features here')
    sl = pyvw.simple_label(2.0, weight=0.5)
    assert sl.label == 2.0
    assert sl.weight == 0.5
    assert sl.prediction == 0.0
    assert sl.initial == 0.0
    assert str(sl) == '2.0:0.5'
    sl2 = pyvw.simple_label(ex)
    assert sl2.label == 1.0
    assert sl2.weight == 1.0
    assert sl2.prediction == 0.0
    assert sl2.initial == 0.0
    assert str(sl2) == '1.0'

def test_multiclass_label():
    vw_ex = vw(quiet=True)
    ex = vw_ex.example('1 |a two features |b more features here')
    ml = pyvw.multiclass_label(2, weight=0.2)
    assert ml.label == 2
    assert ml.weight == 0.2
    assert ml.prediction == 1
    assert str(ml) == '2:0.2'
    ml2 = pyvw.multiclass_label(ex)
    assert ml2.label == 1065353216
    assert ml2.weight == 1.0
    assert ml2.prediction == 0
    assert str(ml2) == '1065353216'

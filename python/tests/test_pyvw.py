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
        model = vw(quiet=True)
        assert not model.finished
        model.finish()
        assert model.finished

    def test_del(self):
        model = vw(quiet=True)
        del model

    def test_oaa(self):
        model = vw(loss_function='logistic', oaa=3, quiet=True)
        model.learn('1 | feature:0')
        model.learn('2 | feature:10')
        assert isinstance(model.predict(' | feature:0', vw.lMulticlass), int)

    def test_oaa_probs(self):
        model = vw(loss_function='logistic', oaa=3, probabilities=True, quiet=True)
        model.learn('1 | feature:0')
        model.learn('2 | feature:10')
        assert isinstance(model.predict(' | feature:0', vw.lMulticlass), list)
        del model

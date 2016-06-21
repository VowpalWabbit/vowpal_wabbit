import pytest
from vowpalwabbit import pyvw


BIT_SIZE = 18


class TestVW:

    @pytest.fixture(scope='function')
    def vw(self):
        return pyvw.vw(quiet=True, b=BIT_SIZE)

    def test_constructor(self, vw):
        assert isinstance(vw, pyvw.vw)

    def test_learn_predict(self, vw):
        ex = vw.example('1 | a b c')
        init = vw.predict(ex)
        assert init == 0
        vw.learn(ex)
        assert vw.predict(ex) > init

    def test_num_weights(self, vw):
        assert vw.num_weights() == 2**BIT_SIZE

    def test_get_weight(self, vw):
        assert vw.get_weight(0, 0) == 0

    def test_finish(self):
        vw = pyvw.vw()
        assert not vw.finished
        vw.finish()
        assert vw.finished

    def test_del(self):
        vw = pyvw.vw()
        del vw

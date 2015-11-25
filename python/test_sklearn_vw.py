from collections import namedtuple
import numpy as np
import pytest
from sklearn_vw import VW, VWClassifier, VWRegressor, tovw
from sklearn import datasets
from sklearn.utils.validation import NotFittedError


"""
Test utilities to support integration of Vowpal Wabbit and scikit-learn
"""

Dataset = namedtuple('Dataset', 'x, y')


@pytest.fixture(scope='module')
def data():
    x, y = datasets.make_hastie_10_2(n_samples=100, random_state=1)
    x = x.astype(np.float32)
    return Dataset(x=x, y=y)


class TestVW:

    def test_init(self):
        assert isinstance(VW(), VW)

    def test_fit(self, data):
        model = VW(loss_function='logistic')
        assert not hasattr(model, 'fit_')

        model.fit(data.x, data.y)
        assert model.fit_

    def test_predict_not_fit(self, data):
        model = VW(loss_function='logistic')
        with pytest.raises(NotFittedError):
            model.predict(data.x[0], data.y[0])

    def test_predict(self, data):
        model = VW(loss_function='logistic')
        model.fit(data.x, data.y)
        assert np.isclose(model.predict(data.x[:1][:1])[0], 0.406929)

    def test_predict_no_convert(self):
        model = VW(loss_function='logistic')
        model.fit(['-1 | bad', '1 | good'], convert_to_vw=False)
        assert np.isclose(model.predict(['| good'], convert_to_vw=False)[0], 0.245515)

    def test_set_params(self):
        model = VW()
        assert 'l' not in model.params

        model.set_params(l=0.1)
        assert model.params['l'] == 0.1

    def test_get_coefs(self, data):
        model = VW()
        model.fit(data.x, data.y)
        weights = model.get_coefs()
        print weights.data
        assert np.allclose(weights.indices, [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 116060])
        assert np.allclose(weights.data, [0.11553502, -0.0166647, -0.00349924, 0.06911729, 0.00252684,
                                          -0.00826817, 0.01991862, -0.02473332, 0.00483846, -0.04616702, -0.00744559])

    def test_get_intercept(self, data):
        model = VW()
        model.fit(data.x, data.y)
        intercept = model.get_intercept()
        assert np.isclose(intercept, -0.00744559)


class TestVWClassifier:

    def test_init(self):
        assert isinstance(VWClassifier(), VWClassifier)

    def test_decision_function(self, data):
        classes = np.array([-1., 1.])
        raw_model = VW(loss_function='logistic')
        raw_model.fit(data.x, data.y)
        predictions = raw_model.predict(data.x)
        class_indices = (predictions > 0).astype(np.int)
        class_predictions = classes[class_indices]

        model = VWClassifier()
        model.fit(data.x, data.y)

        assert np.allclose(class_predictions, model.predict(data.x))


class TestVWRegressor:

    def test_init(self):
        assert isinstance(VWRegressor(), VWRegressor)

    def test_predict(self, data):
        raw_model = VW()
        raw_model.fit(data.x, data.y)

        model = VWRegressor()
        model.fit(data.x, data.y)

        assert np.allclose(raw_model.predict(data.x), model.predict(data.x))


def test_tovw():
    x = np.array([[1.2, 3.4, 5.6], [7.8, 9.10, 11.]])
    y = np.array([1, -1])
    w = [1, 2]

    expected = ['1 1 | 0:1.2 1:3.4 2:5.6',
                '-1 2 | 0:7.8 1:9.1 2:11']

    assert tovw(x=x, y=y, sample_weight=w) == expected
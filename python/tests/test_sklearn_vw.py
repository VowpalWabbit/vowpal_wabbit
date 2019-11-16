import os

from collections import namedtuple
import numpy as np
import pandas as pd
import pytest

from vowpalwabbit.sklearn_vw import VW, VWClassifier, VWRegressor, tovw
from sklearn import datasets
from sklearn.model_selection import KFold
from scipy.sparse import csr_matrix


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

    def test_validate_vw_estimator(self):
        """
        Run VW and VWClassifier through the sklearn estimator validation check

        Note: the VW estimators fail sklearn's estimator validation check. The validator creates a new
        instance of the estimator with the estimator's default args, '--quiet' in VW's case. At some point
        in the validation sequence it calls fit() with some fake data.  The data gets formatted  via tovw() to:

        2 1 | 0:0.5488135039273248 1:0.7151893663724195 2:0.6027633760716439 3:0.5448831829968969 4:0.4236547993389047 5:0.6458941130666561 6:0.4375872112626925 7:0.8917730007820798 8:0.9636627605010293 9:0.3834415188257777

        This gets passed into vw.learn and the python process dies with the error, "Process finished with exit code 139"

        At some point it would probably be worth while figuring out the problem  this and getting the two estimators to
        pass sklearn's validation check
        """

        # check_estimator(VW)
        # check_estimator(VWClassifier)

    def test_init(self):
        assert isinstance(VW(), VW)

    def test_fit(self, data):
        model = VW(loss_function='logistic')
        assert not hasattr(model, 'fit_')

        model.fit(data.x, data.y)
        assert model.fit_

    def test_passes(self, data):
        n_passes = 2
        model = VW(loss_function='logistic', passes=n_passes)
        assert model.passes_ == n_passes

        model.fit(data.x, data.y)
        weights = model.get_coefs()

        model = VW(loss_function='logistic')
        # first pass weights should not be the same
        model.fit(data.x, data.y)
        assert not np.allclose(weights.data, model.get_coefs().data)

    def test_predict_not_fit(self, data):
        model = VW(loss_function='logistic')
        with pytest.raises(ValueError):
            model.predict(data.x[0])

    def test_predict(self, data):
        model = VW(loss_function='logistic')
        model.fit(data.x, data.y)
        assert np.isclose(model.predict(data.x[:1][:1])[0], 0.406929)

    def test_predict_no_convert(self):
        model = VW(loss_function='logistic', convert_to_vw=False)
        model.fit(['-1 | bad', '1 | good'])
        assert np.isclose(model.predict(['| good'])[0], 0.245515)

    def test_set_params(self):
        model = VW()
        assert 'l' not in model.params

        model.set_params(l=0.1)
        assert model.params['l'] == 0.1

        # confirm model params reset with new construction
        model = VW()
        assert 'l' not in model.params

    def test_get_coefs(self, data):
        model = VW()
        model.fit(data.x, data.y)
        weights = model.get_coefs()
        assert np.allclose(weights.indices, [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 116060])

    def test_get_intercept(self, data):
        model = VW()
        model.fit(data.x, data.y)
        intercept = model.get_intercept()
        assert isinstance(intercept, float)

    def test_oaa_probs(self):
        X = ['1 | feature1:2.5',
             '2 | feature1:0.11 feature2:-0.0741',
             '3 | feature3:2.33 feature4:0.8 feature5:-3.1',
             '1 | feature2:-0.028 feature1:4.43',
             '2 | feature5:1.532 feature6:-3.2']
        model = VW(convert_to_vw=False, oaa=3, loss_function='logistic', probabilities=True)
        model.fit(X)
        prediction = model.predict(X)
        assert prediction.shape == [5, 3]
        assert prediction[0, 0] > 0.1

    def test_oaa_probs(self):
        X = ['1 | feature1:2.5',
             '2 | feature1:0.11 feature2:-0.0741',
             '3 | feature3:2.33 feature4:0.8 feature5:-3.1',
             '1 | feature2:-0.028 feature1:4.43',
             '2 | feature5:1.532 feature6:-3.2']
        model = VW(convert_to_vw=False, oaa=3, loss_function='logistic')
        model.fit(X)
        prediction = model.predict(X)
        assert np.allclose(prediction, [1., 2., 3., 1., 2.])

    def test_lrq(self):
        X = ['1 |user A |movie 1',
             '2 |user B |movie 2',
             '3 |user C |movie 3',
             '4 |user D |movie 4',
             '5 |user E |movie 1']
        model = VW(convert_to_vw=False, lrq='um4', lrqdropout=True, loss_function='quantile')
        assert model.params['lrq'] == 'um4'
        assert model.params['lrqdropout']
        model.fit(X)
        prediction = model.predict([' |user C |movie 1'])
        assert np.allclose(prediction, [3.], atol=1)

    def test_bfgs(self):
        data_file = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'resources', 'train.dat')
        model = VW(convert_to_vw=False, oaa=3, passes=30, bfgs=True, data=data_file, cache=True, quiet=False)
        X = ['1 | feature1:2.5',
             '2 | feature1:0.11 feature2:-0.0741',
             '3 | feature3:2.33 feature4:0.8 feature5:-3.1',
             '1 | feature2:-0.028 feature1:4.43',
             '2 | feature5:1.532 feature6:-3.2']
        actual = model.predict(X)
        assert np.allclose(actual, [1.,  2.,  3.,  1.,  2.])

    def test_bfgs_no_data(self):
        with pytest.raises(RuntimeError):
            VW(convert_to_vw=False, oaa=3, passes=30, bfgs=True)

    def test_nn(self):
        vw = VW(convert_to_vw=False, nn=3)
        pos = '1.0 | a b c'
        neg = '-1.0 | d e f'
        vw.fit([pos]*10 + [neg]*10)
        assert vw.predict(['| a b c']) > 0
        assert vw.predict(['| d e f']) < 0


class TestVWClassifier:

    def test_init(self):
        assert isinstance(VWClassifier(), VWClassifier)

    def test_decision_function(self, data):
        classes = np.array([-1., 1.])
        raw_model = VW(loss_function='logistic')
        raw_model.fit(data.x, data.y)
        predictions = raw_model.predict(data.x)
        class_indices = (predictions > 0).astype(np.int)
        expected = classes[class_indices]

        model = VWClassifier()
        model.fit(data.x, data.y)
        actual = model.predict(data.x)

        assert np.allclose(expected, actual)

    def test_shuffle_list(self):
        # dummy data in vw format
        X = ['1 |Pet cat', '-1 |Pet dog', '1 |Pet cat', '1 |Pet cat']

        # Classifier with multiple passes over the data
        clf = VWClassifier(passes=3, convert_to_vw=False)
        clf.fit(X)

        # assert that the dummy data was not perturbed
        assert X == ['1 |Pet cat', '-1 |Pet dog', '1 |Pet cat', '1 |Pet cat']

    def test_shuffle_pd_Series(self):
        # dummy data in vw format
        X = pd.Series(['1 |Pet cat', '-1 |Pet dog', '1 |Pet cat', '1 |Pet cat'], name='catdog')

        kfold = KFold(n_splits=3, random_state=314, shuffle=False)
        for train_idx, valid_idx in kfold.split(X):
            X_train = X[train_idx]
            # Classifier with multiple passes over the data
            clf = VWClassifier(passes=3, convert_to_vw=False)
            # Test that there is no exception raised in the fit on folds
            try:
                clf.fit(X_train)
            except KeyError:
                pytest.fail("Failed the fit over sub-sampled DataFrame")

class TestVWRegressor:

    def test_init(self):
        assert isinstance(VWRegressor(), VWRegressor)

    def test_predict(self, data):
        raw_model = VW()
        raw_model.fit(data.x, data.y)

        model = VWRegressor()
        model.fit(data.x, data.y)

        assert np.allclose(raw_model.predict(data.x), model.predict(data.x))
        # ensure model can make multiple calls to predict
        assert np.allclose(raw_model.predict(data.x), model.predict(data.x))

    def test_delete(self):
        raw_model = VW()
        del raw_model


def test_tovw():
    x = np.array([[1.2, 3.4, 5.6, 1.0, 10], [7.8, 9.10, 11, 0, 20]])
    y = np.array([1, -1])
    w = [1, 2]

    expected = ['1 1 | 0:1.2 1:3.4 2:5.6 3:1 4:10',
                '-1 2 | 0:7.8 1:9.1 2:11 4:20']

    assert tovw(x=x, y=y, sample_weight=w) == expected

    assert tovw(x=csr_matrix(x), y=y, sample_weight=w) == expected

def test_save_load(tmp_path):
    train_file = str(tmp_path / "train.model")

    X = [[1, 2], [3, 4], [5, 6], [7, 8]]
    y = [1, 2, 3, 4]

    model_before = VWRegressor(l=100)
    model_before.fit(X, y)
    before_saving = model_before.predict(X)

    model_before.save(train_file)

    model_after = VWRegressor(l=100)
    model_after.load(train_file)
    after_loading = model_after.predict(X)

    assert all([a == b for a, b in zip(before_saving, after_loading)])

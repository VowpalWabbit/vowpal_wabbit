from collections import namedtuple
import os

import numpy as np
from packaging import version
import pandas as pd
import pytest
from scipy.sparse import csr_matrix
from sklearn import datasets, __version__ as sklearn_version
from sklearn.model_selection import KFold
from sklearn.utils.estimator_checks import check_estimator
from vowpalwabbit.sklearn import (
    VW,
    VWClassifier,
    VWRegressor,
    tovw,
    VWMultiClassifier,
)


"""
Test utilities to support integration of Vowpal Wabbit and scikit-learn
"""

Dataset = namedtuple("Dataset", "x, y")


@pytest.fixture(scope="module")
def data():
    x, y = datasets.make_hastie_10_2(n_samples=100, random_state=1)
    x = x.astype(np.float32)
    return Dataset(x=x, y=y)


def test_tovw():
    x = np.array([[1.2, 3.4, 5.6, 1.0, 10], [7.8, 9.10, 11, 0, 20]])
    y = np.array([2, 0])
    w = [1, 2]

    expected = ["1 1 | 0:1.2 1:3.4 2:5.6 3:1 4:10", "-1 2 | 0:7.8 1:9.1 2:11 4:20"]

    assert tovw(x=x, y=y, sample_weight=w, convert_labels=True) == expected
    assert tovw(x=csr_matrix(x), y=y, sample_weight=w, convert_labels=True) == expected


class BaseVWTest:
    estimator = None

    # must have sklearn version >= 0.22 due to https://github.com/scikit-learn/scikit-learn/issues/6981
    @pytest.mark.skipif(
        version.parse(sklearn_version) < version.parse("0.22"),
        reason="requires sklearn 0.22",
    )
    def test_check_estimator(self):
        # run VW through the sklearn estimator validation check
        # skip check until https://github.com/scikit-learn/scikit-learn/issues/16799 is closed
        return
        check_estimator(self.estimator())

    def test_repr(self):
        model = self.estimator()
        expected = (
            self.estimator.__name__
            + "(convert_labels: True, convert_to_vw: True, passes: 1, quiet: True)"
        )
        assert expected == model.__repr__()


class TestVW(BaseVWTest):
    estimator = VW

    def test_fit(self, data):
        model = VW(loss_function="logistic")
        assert model.vw_ is None

        model.fit(data.x, data.y)
        assert model.vw_ is not None

    def test_save_load(self, data):
        file_name = "tmp_sklearn.model"

        model_before = VW(l=100)
        model_before.fit(data.x, data.y)
        before_saving = model_before.predict(data.x)
        model_before.save(file_name)

        model_after = VW(l=100)
        model_after.load(file_name)
        after_loading = model_after.predict(data.x)

        assert np.allclose(before_saving, after_loading)

    def test_passes(self, data):
        n_passes = 2
        model = VW(loss_function="logistic", passes=n_passes)
        assert getattr(model, "passes") == n_passes

        model.fit(data.x, data.y)
        weights = model.get_coefs()

        model = VW(loss_function="logistic")
        # first pass weights should not be the same
        model.fit(data.x, data.y)
        assert not np.allclose(weights.data, model.get_coefs().data)

    def test_predict(self, data):
        model = VW(loss_function="logistic")
        model.fit(data.x, data.y)
        actual = model.predict(data.x[:1][:1])[0]
        assert np.isclose(actual, 0.406929, atol=1e-2)

    def test_predict_no_convert(self):
        model = VW(loss_function="logistic", convert_to_vw=False)
        model.fit(["-1 | bad", "1 | good"])
        actual = model.predict(["| good"])[0]
        assert np.isclose(actual, 0.245515, atol=1e-2)

    def test_set_params(self):
        model = VW()
        assert getattr(model, "l") is None

        model.set_params(l=0.1)
        assert getattr(model, "l") == 0.1
        assert getattr(model, "vw_") is None

        # confirm model params reset with new construction
        model = VW()
        assert getattr(model, "l") is None

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

    def test_oaa(self):
        X = [
            "1 | feature1:2.5",
            "2 | feature1:0.11 feature2:-0.0741",
            "3 | feature3:2.33 feature4:0.8 feature5:-3.1",
            "1 | feature2:-0.028 feature1:4.43",
            "2 | feature5:1.532 feature6:-3.2",
        ]
        model = VW(convert_to_vw=False, oaa=3, loss_function="logistic")
        model.fit(X)
        prediction = model.predict(X)
        assert np.allclose(prediction, [1.0, 2.0, 3.0, 1.0, 2.0])

    def test_oaa_probs(self):
        X = [
            "1 | feature1:2.5",
            "2 | feature1:0.11 feature2:-0.0741",
            "3 | feature3:2.33 feature4:0.8 feature5:-3.1",
            "1 | feature2:-0.028 feature1:4.43",
            "2 | feature5:1.532 feature6:-3.2",
        ]
        model = VW(
            convert_to_vw=False, oaa=3, loss_function="logistic", probabilities=True
        )
        model.fit(X)
        prediction = model.predict(X)
        assert prediction.shape[0] == 5
        assert prediction.shape[1] == 3
        assert prediction[0, 0] > 0.1

    def test_lrq(self):
        X = [
            "1 |user A |movie 1",
            "2 |user B |movie 2",
            "3 |user C |movie 3",
            "4 |user D |movie 4",
            "5 |user E |movie 1",
        ]
        model = VW(
            convert_to_vw=False, lrq="um4", lrqdropout=True, loss_function="quantile"
        )
        assert getattr(model, "lrq") == "um4"
        assert getattr(model, "lrqdropout")
        model.fit(X)
        prediction = model.predict([" |user C |movie 1"])
        assert np.allclose(prediction, [3.0], atol=1)

    def test_bfgs(self):
        data_file = os.path.join(
            os.path.dirname(os.path.realpath(__file__)), "resources", "train.dat"
        )
        model = VW(
            convert_to_vw=False,
            oaa=3,
            passes=30,
            bfgs=True,
            data=data_file,
            cache=True,
            quiet=False,
        )
        model.fit()
        X = [
            "1 | feature1:2.5",
            "2 | feature1:0.11 feature2:-0.0741",
            "3 | feature3:2.33 feature4:0.8 feature5:-3.1",
            "1 | feature2:-0.028 feature1:4.43",
            "2 | feature5:1.532 feature6:-3.2",
        ]
        actual = model.predict(X)
        assert np.allclose(actual, [1.0, 2.0, 3.0, 1.0, 2.0])

    def test_bfgs_no_data(self):
        with pytest.raises(RuntimeError):
            VW(convert_to_vw=False, oaa=3, passes=30, bfgs=True).fit()

    def test_nn(self):
        vw = VW(convert_to_vw=False, nn=3)
        pos = "1.0 | a b c"
        neg = "-1.0 | d e f"
        vw.fit([pos] * 10 + [neg] * 10)
        assert vw.predict(["| a b c"]) > 0
        assert vw.predict(["| d e f"]) < 0

    def test_del(self, data):
        model = VW()
        model.fit(data.x, data.y)
        del model


class TestVWClassifier(BaseVWTest):
    estimator = VWClassifier

    def test_decision_function(self, data):
        model = VWClassifier()
        model.fit(data.x, data.y)
        actual = model.decision_function(data.x)
        assert actual.shape[0] == 100
        assert np.isclose(actual[0], 0.4069, atol=1e-2)

    def test_predict_proba(self, data):
        model = VWClassifier()
        model.fit(data.x, data.y)
        actual = model.predict_proba(data.x)
        assert actual.shape[0] == 100
        assert np.allclose(actual[0], [0.3997, 0.6003], atol=1e-2)

    def test_repr(self):
        model = VWClassifier()
        expected = "VWClassifier(convert_labels: True, convert_to_vw: True, loss_function: logistic, passes: 1, quiet: True)"
        assert expected == model.__repr__()

    def test_shuffle_list(self):
        # dummy data in vw format
        X = ["1 |Pet cat", "-1 |Pet dog", "1 |Pet cat", "1 |Pet cat"]

        # Classifier with multiple passes over the data
        clf = VWClassifier(passes=3, convert_to_vw=False)
        clf.fit(X)

        # assert that the dummy data was not perturbed
        assert X == ["1 |Pet cat", "-1 |Pet dog", "1 |Pet cat", "1 |Pet cat"]

    def test_shuffle_pd_series(self):
        # dummy data in vw format
        X = pd.Series(
            ["1 |Pet cat", "-1 |Pet dog", "1 |Pet cat", "1 |Pet cat"], name="catdog"
        )

        kfold = KFold(n_splits=3, random_state=314, shuffle=True)
        for train_idx, valid_idx in kfold.split(X):
            X_train = X[train_idx]
            # Classifier with multiple passes over the data
            clf = VWClassifier(passes=3, convert_to_vw=False)
            # Test that there is no exception raised in the fit on folds
            try:
                clf.fit(X_train)
            except KeyError:
                pytest.fail("Failed the fit over sub-sampled DataFrame")


class TestVWRegressor(BaseVWTest):
    estimator = VWRegressor

    def test_predict(self, data):
        raw_model = VW()
        raw_model.fit(data.x, data.y)

        model = VWRegressor()
        model.fit(data.x, data.y)

        assert np.allclose(raw_model.predict(data.x), model.predict(data.x))
        # ensure model can make multiple calls to predict
        assert np.allclose(raw_model.predict(data.x), model.predict(data.x))

    def test_repr(self):
        model = self.estimator()
        expected = (
            self.estimator.__name__
            + "(convert_labels: False, convert_to_vw: True, passes: 1, quiet: True)"
        )
        assert expected == model.__repr__()


class TestVWMultiClassifier(BaseVWTest):

    estimator = VWMultiClassifier

    def test_predict_proba(self, data):
        model = VWMultiClassifier(oaa=2, loss_function="logistic")
        model.fit(data.x, data.y)
        actual = model.predict_proba(data.x)
        assert actual.shape == (100, 2)
        expected = [0.8967, 0.1032]
        assert np.allclose(actual[0], expected, atol=1e-2)

    def test_predict(self, data):
        model = VWMultiClassifier(oaa=2, loss_function="logistic")
        model.fit(data.x, data.y)
        actual = model.predict(data.x)
        assert actual.shape == (100,)
        assert all([x in [-1, 1] for x in actual])

    def test_repr(self):
        model = VWMultiClassifier()
        expected = "VWMultiClassifier(convert_labels: True, convert_to_vw: True, loss_function: logistic, passes: 1, probabilities: True, quiet: True)"
        assert expected == model.__repr__()

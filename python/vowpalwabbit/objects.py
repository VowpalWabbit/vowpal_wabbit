# -*- coding: utf-8 -*-
# pylint: unused-argument, invalid-name, too-many-arguments, too-many-locals
# Objects similar to private objects in https://github.com/scikit-learn/scikit-learn/blob/master/sklearn/linear_model/_base.py
"""Models to replicate private methods used from sklearn module"""

import numpy as np
import scipy.sparse as sp
from scipy.special import expit
from sklearn.base import ClassifierMixin
from sklearn.utils.extmath import safe_sparse_dot
from sklearn.utils.validation import (check_is_fitted,
                                      check_array)


class LinearClassifierMixin(ClassifierMixin):
    """Mixin for linear classifiers.
    Handles prediction for sparse and dense X.
    """

    def decision_function(self, X):
        """
        Predict confidence scores for samples.
        The confidence score for a sample is the signed distance of that
        sample to the hyperplane.
        Parameters
        ----------
        X : array_like or sparse matrix, shape (n_samples, n_features)
            Samples.
        Returns
        -------
        array, shape=(n_samples,) if n_classes == 2 else (n_samples, n_classes)
            Confidence scores per (sample, class) combination. In the binary
            case, confidence score for self.classes_[1] where >0 means this
            class would be predicted.
        """
        check_is_fitted(self)

        X = check_array(X, accept_sparse='csr')

        n_features = self.coef_.shape[1]
        if X.shape[1] != n_features:
            raise ValueError("X has %d features per sample; expecting %d"
                             % (X.shape[1], n_features))

        scores = safe_sparse_dot(X, self.coef_.T,
                                 dense_output=True) + self.intercept_
        return scores.ravel() if scores.shape[1] == 1 else scores
    
    def predict(self, X):
        """
        Predict class labels for samples in X.
        Parameters
        ----------
        X : array_like or sparse matrix, shape (n_samples, n_features)
            Samples.
        Returns
        -------
        C : array, shape [n_samples]
            Predicted class label per sample.
        """
        scores = self.decision_function(X)
        if len(scores.shape) == 1:
            indices = (scores > 0).astype(np.int)
        else:
            indices = scores.argmax(axis=1)
        return self.classes_[indices]

    def _predict_proba_lr(self, X):
        """
        Probability estimation for OvR logistic regression.
        Positive class probabilities are computed as
        1. / (1. + np.exp(-self.decision_function(X)));
        multiclass is handled by normalizing that over all classes.
        """
        prob = self.decision_function(X)
        expit(prob, out=prob)
        if prob.ndim == 1:
            return np.vstack([1 - prob, prob]).T
        else:
            # OvR normalization, like LibLinear's predict_probability
            prob /= prob.sum(axis=1).reshape((prob.shape[0], -1))
            return prob

class SparseCoefMixin:
    """
    Mixin for converting coef_ to and from CSR format.
    L1-regularizing estimators should inherit this.
    """

    def densify(self):
        """
        Convert coefficient matrix to dense array format.
        Converts the ``coef_`` member (back) to a numpy.ndarray. This is the
        default format of ``coef_`` and is required for fitting, so calling
        this method is only required on models that have previously been
        sparsified; otherwise, it is a no-op.
        Returns
        -------
        self
            Fitted estimator.
        """
        msg = "Estimator, %(name)s, must be fitted before densifying."
        check_is_fitted(self, msg=msg)
        if sp.issparse(self.coef_):
            self.coef_ = self.coef_.toarray()
        return self

    def sparsify(self):
        """
        Convert coefficient matrix to sparse format.
        Converts the ``coef_`` member to a scipy.sparse matrix, which for
        L1-regularized models can be much more memory- and storage-efficient
        than the usual numpy.ndarray representation.
        The ``intercept_`` member is not converted.
        Returns
        -------
        self
            Fitted estimator.
        Notes
        -----
        For non-sparse models, i.e. when there are not many zeros in ``coef_``,
        this may actually *increase* memory usage, so use this method with
        care. A rule of thumb is that the number of zero elements, which can
        be computed with ``(coef_ == 0).sum()``, must be more than 50% for this
        to provide significant benefits.
        After calling this method, further fitting with the partial_fit
        method (if any) will not work until you call densify.
        """
        msg = "Estimator, %(name)s, must be fitted before sparsifying."
        check_is_fitted(self, msg=msg)
        self.coef_ = sp.csr_matrix(self.coef_)
        return self
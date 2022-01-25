# -*- coding: utf-8 -*-
# pylint: unused-argument, invalid-name, too-many-arguments, too-many-locals
"""Utilities to support integration of Vowpal Wabbit and scikit-learn"""

import io
import os
import re
from tempfile import NamedTemporaryFile
from typing import Dict

import numpy as np
from scipy.sparse import csr_matrix
from scipy.special import logit
from sklearn.exceptions import NotFittedError
from sklearn.base import BaseEstimator, RegressorMixin
from sklearn.utils.extmath import log_logistic
from sklearn.linear_model import LogisticRegression
from sklearn.datasets import dump_svmlight_file
from sklearn.utils import check_array, check_X_y, shuffle
from vowpalwabbit import Workspace

DEFAULT_NS = ""
CONSTANT_HASH = 116060
INVALID_CHARS = re.compile(r"[\|: \n]+")


class VW(BaseEstimator):
    """Vowpal Wabbit Scikit-learn Base Estimator wrapper"""

    convert_to_vw: bool = True
    """flag to convert X input to vw format"""
    convert_labels: bool = True
    """Convert labels of the form [0,1] to [-1,1]"""
    vw_: Workspace = None

    def __init__(
        self,
        convert_to_vw=True,
        convert_labels=True,
        ring_size=None,
        strict_parse=None,
        learning_rate=None,
        l=None,
        power_t=None,
        decay_learning_rate=None,
        initial_t=None,
        feature_mask=None,
        initial_regressor=None,
        i=None,
        initial_weight=None,
        random_weights=None,
        normal_weights=None,
        truncated_normal_weights=None,
        sparse_weights=None,
        input_feature_regularizer=None,
        quiet=True,
        random_seed=None,
        hash=None,
        hash_seed=None,
        ignore=None,
        ignore_linear=None,
        keep=None,
        redefine=None,
        bit_precision=None,
        b=None,
        noconstant=None,
        constant=None,
        C=None,
        ngram=None,
        skips=None,
        feature_limit=None,
        affix=None,
        spelling=None,
        dictionary=None,
        dictionary_path=None,
        interactions=None,
        permutations=None,
        leave_duplicate_interactions=None,
        quadratic=None,
        q=None,
        cubic=None,
        testonly=None,
        t=None,
        holdout_off=None,
        holdout_period=None,
        holdout_after=None,
        early_terminate=None,
        passes=1,
        initial_pass_length=None,
        examples=None,
        min_prediction=None,
        max_prediction=None,
        sort_features=None,
        loss_function=None,
        quantile_tau=None,
        l1=None,
        l2=None,
        no_bias_regularization=None,
        named_labels=None,
        final_regressor=None,
        f=None,
        readable_model=None,
        invert_hash=None,
        save_resume=None,
        preserve_performance_counters=None,
        output_feature_regularizer_binary=None,
        output_feature_regularizer_text=None,
        oaa=None,
        ect=None,
        csoaa=None,
        wap=None,
        probabilities=None,
        nn=None,
        inpass=None,
        multitask=None,
        dropout=None,
        meanfield=None,
        conjugate_gradient=None,
        bfgs=None,
        hessian_on=None,
        mem=None,
        termination=None,
        lda=None,
        lda_alpha=None,
        lda_rho=None,
        lda_D=None,
        lda_epsilon=None,
        minibatch=None,
        svrg=None,
        stage_size=None,
        ftrl=None,
        coin=None,
        pistol=None,
        ftrl_alpha=None,
        ftrl_beta=None,
        ksvm=None,
        kernel=None,
        bandwidth=None,
        degree=None,
        sgd=None,
        adaptive=None,
        invariant=None,
        normalized=None,
        link=None,
        stage_poly=None,
        sched_exponent=None,
        batch_sz=None,
        batch_sz_no_doubling=None,
        lrq=None,
        lrqdropout=None,
        lrqfa=None,
        data=None,
        d=None,
        cache=None,
        c=None,
        cache_file=None,
        json=None,
        kill_cache=None,
        k=None,
    ):
        """VW model constructor, exposing all supported parameters to keep sklearn happy

        Args:
            convert_to_vw (bool): flag to convert X input to vw format
            convert_labels (bool): Convert labels of the form [0,1] to [-1,1]
            ring_size (int): size of example ring
            strict_parse (bool): throw on malformed examples
            learning_rate,l (float): Set learning rate
            power_t (float): t power value
            decay_learning_rate (float): Set Decay factor for learning_rate between passes
            initial_t (float): initial t value
            feature_mask (str): Use existing regressor to determine which parameters may be updated.
                If no initial_regressor given, also used for initial weights.
            initial_regressor,i (str): Initial regressor(s)
            initial_weight (float): Set all weights to an initial value of arg.
            random_weights (bool): make initial weights random
            normal_weights (bool): make initial weights normal
            truncated_normal_weights (bool): make initial weights truncated normal
            sparse_weights (float): Use a sparse datastructure for weights
            input_feature_regularizer (str): Per feature regularization input file
            quiet (bool): Don't output disgnostics and progress updates
            random_seed (integer): seed random number generator
            hash (str): , all
            hash_seed (int): seed for hash function
            ignore (str): ignore namespaces beginning with character <arg>
            ignore_linear (str): ignore namespaces beginning with character <arg> for linear terms only
            keep (str): keep namespaces beginning with character <arg>
            redefine (str): Redefine namespaces beginning with characters of string S as namespace N. <arg> shall be in
                form 'N:=S' where := is operator. Empty N or S are treated as default namespace.
                Use ':' as a wildcard in S.
            bit_precision,b (integer): number of bits in the feature table
            noconstant (bool): Don't add a constant feature
            constant,C (float): Set initial value of constant
            ngram (str): Generate N grams. To generate N grams for a single namespace 'foo', arg should be fN.
            skips (str): Generate skips in N grams. This in conjunction with the ngram tag can be used to generate
                generalized n-skip-k-gram. To generate n-skips for a single namespace 'foo', arg should be fN.
            feature_limit (str): limit to N features. To apply to a single namespace 'foo', arg should be fN
            affix (str): generate prefixes/suffixes of features; argument '+2a,-3b,+1' means generate 2-char prefixes for
                namespace a, 3-char suffixes for b and 1 char prefixes for default namespace
            spelling (str): compute spelling features for a give namespace (use '_' for default namespace)
            dictionary (str): read a dictionary for additional features (arg either 'x:file' or just 'file')
            dictionary_path (str): look in this directory for dictionaries; defaults to current directory or env{PATH}
            interactions (str): Create feature interactions of any level between namespaces.
            permutations (bool): Use permutations instead of combinations for feature interactions of same namespace.
            leave_duplicate_interactions (bool): Don't remove interactions with duplicate combinations of namespaces. For
                ex. this is a duplicate: '-q ab -q ba' and a lot more in '-q ::'.
            quadratic,q (str): Create and use quadratic features, q:: corresponds to a wildcard for all printable characters
            cubic (str): Create and use cubic features
            testonly,t (bool): Ignore label information and just test
            holdout_off (bool): no holdout data in multiple passes
            holdout_period (int): holdout period for test only
            holdout_after (int): holdout after n training examples
            early_terminate (int): Specify the number of passes tolerated when holdout loss doesn't
                decrease before early termination
            passes (int): Number of Training Passes
            initial_pass_length (int): initial number of examples per pass
            examples (int): number of examples to parse
            min_prediction (float): Smallest prediction to output
            max_prediction (float): Largest prediction to output
            sort_features (bool): turn this on to disregard order in which features have been defined. This will lead to
                smaller cache sizes
            loss_function (str): default_value("squared"), "Specify the loss function to be used, uses squared by default.
                Currently available ones are squared, classic, hinge, logistic and quantile.
            quantile_tau (float): Parameter \\tau associated with Quantile loss. Defaults to 0.5
            l1 (float): l_1 lambda (L1 regularization)
            l2 (float): l_2 lambda (L2 regularization)
            no_bias_regularization (bool): no bias in regularization
            named_labels (str): use names for labels (multiclass, etc.) rather than integers, argument specified all
                possible labels, comma-sep, eg \"--named_labels Noun,Verb,Adj,Punc\"
            final_regressor,f (str): Final regressor
            readable_model (str): Output human-readable final regressor with numeric features
            invert_hash (str): Output human-readable final regressor with feature names.  Computationally expensive.
            save_resume (bool): save extra state so learning can be resumed later with new data
            preserve_performance_counters (bool): reset performance counters when warmstarting
            output_feature_regularizer_binary (str): Per feature regularization output file
            output_feature_regularizer_text (str): Per feature regularization output file, in text
            oaa (integer): Use one-against-all multiclass learning with labels
            oaa_subsample (int): subsample this number of negative examples when learning
            ect (integer): Use error correcting tournament multiclass learning
            csoaa (integer): Use cost sensitive one-against-all multiclass learning
            wap (integer): Use weighted all pairs multiclass learning
            probabilities (float): predict probabilities of all classes
            nn (integer): Use a sigmoidal feed-forward neural network with N hidden units
            inpass (bool): Train or test sigmoidal feed-forward network with input pass-through
            multitask (bool): Share hidden layer across all reduced tasks
            dropout (bool): Train or test sigmoidal feed-forward network using dropout
            meanfield (bool): Train or test sigmoidal feed-forward network using mean field
            conjugate_gradient (bool): use conjugate gradient based optimization
            bgfs (bool): use bfgs updates
            hessian_on (bool): use second derivative in line search
            mem (int): memory in bfgs
            termination (float): termination threshold
            lda (int): Run lda with <int> topics
            lda_alpha (float): Prior on sparsity of per-document topic weights
            lda_rho (float): Prior on sparsity of topic distributions
            lda_D (int): Number of documents
            lda_epsilon (float): Loop convergence threshold
            minibatch (int): Minibatch size for LDA
            svrg (bool): Streaming Stochastic Variance Reduced Gradient
            stage_size (int): Number of passes per SVRG stage
            ftrl (bool): Run Follow the Proximal Regularized Leader
            coin (bool): Coin betting optimizer
            pistol (bool): PiSTOL - Parameter free STOchastic Learning
            ftrl_alpha (float): Alpha parameter for FTRL optimization
            ftrl_beta (float): Beta parameters for FTRL optimization
            ksvm (bool): kernel svm
            kernel (str): type of kernel (rbf or linear (default))
            bandwidth (int): bandwidth of rbf kernel
            degree (int): degree of poly kernel
            sgd (bool): use regular stochastic gradient descent update
            adaptive (bool): use adaptive, individual learning rates
            adax (bool): use adaptive learning rates with x^2 instead of g^2x^2
            invariant (bool): use save/importance aware updates
            normalized (bool): use per feature normalized updates
            link (str): Specify the link function - identity, logistic, glf1 or poisson
            stage_poly (bool): use stagewise polynomial feature learning
            sched_exponent (int): exponent controlling quantity of included features
            batch_sz (int): multiplier on batch size before including more features
            batch_sz_no_doubling (bool): batch_sz does not double
            lrq (bool): use low rank quadratic features
            lrqdropout (bool): use dropout training for low rank quadratic features
            lrqfa (bool): use low rank quadratic features with field aware weights
            data,d (str): path to data file for fitting external to sklearn
            cache,c (str): use a cache. default is <data>.cache
            cache_file (str): path to cache file to use
            json (bool): enable JSON parsing
            kill_cache, k (bool): do not reuse existing cache file, create a new one always
        """

        for k, v in dict(locals()).items():
            if k != "self" and not k.endswith("_"):
                setattr(self, k, v)

        super(VW, self).__init__()

    def get_vw(self):
        """Get the vw instance

        Returns:
            vowpalwabbit.Workspace: instance
        """
        return self.vw_

    def fit(self, X=None, y=None, sample_weight=None):
        """Fit the model according to the given training data

        TODO:
            For first pass create and store example objects. For N-1 passes use example objects
            directly (simulate cache file...but in memory for faster processing)

        Args:
            X : {array-like, sparse matrix}, shape (n_samples, n_features or 1 if not convert_to_vw) or
                Training vector, where n_samples in the number of samples and
                n_features is the number of features.
                if not using convert_to_vw, X is expected to be a list of vw formatted feature vector strings with labels
            y : array-like, shape (n_samples,), optional if not convert_to_vw
                Target vector relative to X.
            sample_weight : array-like, shape (n_samples,)
                            sample weight vector relative to X.

        Returns:
            self
        """

        params = {k: v for k, v in self.get_params().items() if v is not None}

        passes = 1
        use_data_file = params.get("data", params.get("d", False))
        if not use_data_file:
            # remove passes from vw params since we're feeding in the data manually
            passes = params.pop("passes", passes)
            if params.get("bfgs", False):
                raise RuntimeError(
                    "An external data file must be used to fit models using the bfgs option"
                )

        # remove estimator attributes from vw params
        for key in self._get_est_params():
            params.pop(key, None)

        # add vw attributes
        params.update(self._get_vw_params())

        self.vw_ = Workspace(**params)

        if X is not None:
            if self.convert_to_vw:
                X = tovw(
                    x=X,
                    y=y,
                    sample_weight=sample_weight,
                    convert_labels=self.convert_labels,
                )

            # add examples to model
            for n in range(passes):
                if n >= 1:
                    examples = shuffle(X)
                else:
                    examples = X
                for idx, example in enumerate(examples):
                    self.vw_.learn(example)

        return self

    def predict(self, X):
        """Predict with Vowpal Wabbit model

        Args:
            X ({array-like, sparse matrix}, shape \(n_samples, n_features or 1\)):
                Training vector, where n_samples in the number of samples and
                n_features is the number of features.
                if not using convert_to_vw, X is expected to be a list of vw formatted feature vector strings with labels

        Returns:
            array-like, shape (n_samples, 1 or n_classes): y. Output vector relative to X.
        """

        # check_is_fitted
        if self.vw_ is None:
            msg = (
                "This %(name)s instance is not fitted yet. Call 'fit' with "
                "appropriate arguments before using this method."
            )
            raise NotFittedError(msg % {"name": self.__class__.__name__})

        if self.convert_to_vw:
            X = tovw(X)

        if hasattr(X, "__len__"):
            num_samples = len(X)
        elif hasattr(X, "__shape__"):
            num_samples = X.shape[0]
        elif hasattr(X, "__array__"):
            num_samples = np.asarray(X).shape[0]
        else:
            raise Exception("Invalid type for input X")

        model = self.get_vw()

        shape = [num_samples]
        classes = getattr(self, "classes_", None)
        if classes is None:
            for estimator in ["csoaa", "ect", "oaa", "wap"]:
                n_classes = getattr(self, estimator)
                if n_classes is not None:
                    break
        else:
            n_classes = len(classes)
        if n_classes is not None and getattr(self, "probabilities", False):
            shape.append(n_classes)
        y = np.empty(shape)

        # predict examples
        for idx, x in enumerate(X):
            y[idx] = model.predict(ec=x)

        return y

    def get_params(self, deep=True):
        """This returns the full set of vw and estimator parameters currently in use"""
        return {k: v for k, v in vars(self).items() if k != "vw_"}

    def set_params(self, **kwargs):
        """This destroys and recreates the Vowpal Wabbit model with updated parameters
        any parameters not provided will remain as they are currently"""

        params = self.get_params()
        params.update(kwargs)
        private_params = dict()
        for k, v in params.copy().items():
            if k.endswith("_"):
                private_params[k] = v
                params.pop(k)
        self.__init__(**params)
        for k, v in private_params.items():
            setattr(self, k, v)
        self.vw_ = None
        return self

    def get_coefs(self):
        """Returns coefficient weights as ordered sparse matrix

        Returns:
            sparse matrix: coefficient weights for model
        """

        model = self.get_vw()
        return csr_matrix([model.get_weight(i) for i in range(model.num_weights())])

    def set_coefs(self, coefs):
        """Sets coefficients weights from ordered sparse matrix

        Args:
            coefs (sparse matrix): coefficient weights for model
        """

        model = self.get_vw()
        for i in range(coefs.getnnz()):
            model.set_weight(int(coefs.indices[i]), 0, float(coefs.data[i]))

    def get_intercept(self):
        """Returns intercept weight for model

        Returns:
            int: intercept value. 0 if no constant
        """

        return self.get_vw().get_weight(CONSTANT_HASH)

    def save(self, filename):
        """Save model to file"""
        model = self.get_vw()
        model.save(filename=filename)

    def load(self, filename):
        """Load model from file"""
        self.set_params(initial_regressor=filename)
        self.fit()
        setattr(self, "initial_regressor", None)

    def _get_est_params(self):
        """This returns only the set of estimator parameters currently in use"""
        return dict(
            convert_labels=self.convert_labels, convert_to_vw=self.convert_to_vw
        )

    def _get_vw_params(self):
        """This returns specific vw parameters to inject at fit"""
        return dict()

    def __del__(self):
        self.vw_ = None

    def __repr__(self, **kwargs):
        vw_vars = sorted((k, v) for k, v in vars(self).items() if v is not None)
        items = ["{i[0]}: {i[1]}".format(i=i) for i in vw_vars]
        return "{}({})".format(self.__class__.__name__, ", ".join(items))

    def __getstate__(self):
        """Support pickling"""
        f = NamedTemporaryFile()
        self.save(filename=f.name)
        state = self.get_params()
        with open(f.name, "rb") as tmp:
            state["vw_"] = tmp.read()
        f.close()
        return state

    def __setstate__(self, state):
        """Support unpickling"""
        f = NamedTemporaryFile(delete=False)
        f.write(state.pop("vw_"))
        f.close()
        self.set_params(**state)
        self.load(filename=f.name)
        os.unlink(f.name)


class LinearClassifierMixin(LogisticRegression):
    def __init__(self):
        # overwrite init here so base class inits aren't used
        pass


class VWClassifier(VW, LinearClassifierMixin):
    """Vowpal Wabbit Classifier model for binary classification
    Use VWMultiClassifier for multiclass classification
    Note - We are assuming the VW.predict returns logits, applying link=logistic will break this assumption
    """

    coef_ = None
    """Empty sparse matrix used the check if model has been fit"""
    classes_ = np.array([-1.0, 1.0])
    """Binary class labels"""

    def __init__(self, loss_function="logistic", **kwargs):
        kwargs["loss_function"] = loss_function
        super(VWClassifier, self).__init__(**kwargs)

    def fit(self, X=None, y=None, sample_weight=None):
        """
        Fit the model according to the given training data.

        Args:
            X : {array-like, sparse matrix} of shape (n_samples, n_features)
                Training vector, where n_samples is the number of samples and
                n_features is the number of features.
            y : array-like of shape (n_samples,)
                Target vector relative to X.
            sample_weight : array-like of shape (n_samples,) default=None
                Array of weights that are assigned to individual samples.
                If not provided, then each sample is given unit weight.

        Returns:
            self
        """

        # this attribute is used to check fitted in sparsify()
        if self.coef_ is None:
            self.coef_ = csr_matrix([])
            if y is not None:
                y = check_array(y, ensure_2d=False, dtype=None, accept_sparse=True)
                self.classes_, y = np.unique(y, return_inverse=True)
            # TODO: raise error once check_sparsify_coefficients respects binary_only flag
            # if self._more_tags.get('binary_only') and len(self.classes_) != 2:
            #    raise Exception('VWClassifier can only be used for binary classification')
        return VW.fit(self, X=X, y=y, sample_weight=sample_weight)

    def decision_function(self, X):
        """
        Predict confidence scores for samples.
        The confidence score for a sample is the signed distance of that
        sample to the hyperplane.

        Args:
            X : array_like or sparse matrix, shape (n_samples, n_features)
                Samples.

        Returns:
            array, shape=(n_samples,) if n_classes == 2 else (n_samples, n_classes)
                Confidence scores per (sample, class) combination. In the binary
                case, confidence score for self.classes_[1] where >0 means this
                class would be predicted.
        """

        return VW.predict(self, X=X)

    def predict(self, X):
        """
        Predict class labels for samples in X.

        Args:
            X : array_like or sparse matrix, shape (n_samples, n_features)
                Samples.

        Returns:
            array, shape [n_samples] : C. Predicted class label per sample.
        """

        scores = self.decision_function(X)
        if len(scores.shape) == 1:
            indices = (scores > 0).astype(np.int)
        else:
            indices = scores.argmax(axis=1)
        return self.classes_[indices]

    def predict_proba(self, X):
        """Predict probabilities for samples

        Args:
            X : {array-like, sparse matrix}, shape = (n_samples, n_features)
                Samples.

        Returns:
            array-like of shape (n_samples, n_classes): T. Returns the probability of the sample for each class in the model,
                where classes are ordered as they are in ``self.classes_``.
        """

        probs = np.exp(log_logistic(self.decision_function(X)))
        return np.column_stack((1 - probs, probs))

    def _get_est_params(self):
        """This returns only the set of estimator parameters currently in use"""
        params = VW._get_est_params(self)
        params.update(dict(classes_=self.classes_, coef_=self.coef_))
        return params

    def _more_tags(self):
        # disable validation to allow for features to differ between fit and predict
        return dict(binary_only=True, requires_fit=True, no_validation=True)


class VWRegressor(VW, RegressorMixin):
    """Vowpal Wabbit Regressor model"""

    def __init__(self, convert_labels=False, **kwargs):
        kwargs["convert_labels"] = convert_labels
        super(VWRegressor, self).__init__(**kwargs)

    def _more_tags(self):
        return dict(poor_score=True)


class VWMultiClassifier(VWClassifier):
    """Vowpal Wabbit MultiClassifier model
    Note - We are assuming the VW.predict returns probabilities, setting probabilities=False will break this assumption
    """

    classes_ = None
    """Class labels"""
    estimator_ = None
    """"type of estimator to use [csoaa, ect, oaa, wap] and number of classes"""

    def __init__(self, probabilities=True, **kwargs):
        kwargs["probabilities"] = probabilities
        super(VWMultiClassifier, self).__init__(**kwargs)

    def fit(self, X=None, y=None, sample_weight=None):
        """
        Fit the model according to the given training data.

        Args:
            X : {array-like, sparse matrix} of shape (n_samples, n_features)
                Training vector, where n_samples is the number of samples and
                n_features is the number of features.
            y : array-like of shape (n_samples,)
                Target vector relative to X.
            sample_weight : array-like of shape (n_samples,) default=None
                Array of weights that are assigned to individual samples.
                If not provided, then each sample is given unit weight.

        Returns:
            self
        """

        # this attribute is used to check fitted in sparsify()
        if self.coef_ is None:
            self.coef_ = csr_matrix([])

            if y is not None:
                y = check_array(y, ensure_2d=False, dtype=None, accept_sparse=True)
                self.classes_, y = np.unique(y, return_inverse=True)

            # must set multiclass estimator
            for estimator in ["csoaa", "ect", "oaa", "wap"]:
                n_classes = getattr(self, estimator)
                self.estimator_ = {estimator: n_classes}
                if n_classes is not None:
                    if self.classes_ is None:
                        self.classes_ = np.array(list(range(1, n_classes + 1))).astype(
                            "int"
                        )
                    break
            else:
                # use oaa by default and determine classes from y
                self.estimator_ = dict(oaa=len(self.classes_))

        return VW.fit(self, X=X, y=y, sample_weight=sample_weight)

    def decision_function(self, X):
        """
        Predict confidence scores for samples.
        The confidence score for a sample is the signed distance of that
        sample to the hyperplane.

        Args:
            X : array_like or sparse matrix, shape (n_samples, n_features)
                Samples.

        Returns:
            array, shape=(n_samples, n_classes): Confidence scores per (sample, class) combination.
        """

        logits = logit(VW.predict(self, X=X))
        if logits.shape[1] == 2:
            logits = logits[:, 1]
        return logits

    def predict_proba(self, X):
        """Predict probabilities for each class.

        Args:
            X : {array-like, sparse matrix}, shape = (n_samples, n_features)
                Samples.

        Returns:
            array, shape=(n_samples,) if n_classes == 2 else (n_samples, n_classes)
                Confidence scores per (sample, class) combination. In the binary
                case, confidence score for self.classes_[1] where >0 means this
                class would be predicted.

        Examples:
            >>> import numpy as np
            >>> X = np.array([ [10, 10], [8, 10], [-5, 5.5], [-5.4, 5.5], [-20, -20],  [-15, -20] ])
            >>> y = np.array([1, 1, 2, 2, 3, 3])
            >>> from vowpalwabbit.sklearn import VWMultiClassifier
            >>> model = VWMultiClassifier(oaa=3, loss_function='logistic')
            >>> _ = model.fit(X, y)
            >>> model.predict_proba(X)
            array([[0.38928846, 0.30534211, 0.30536944],
                   [0.40664235, 0.29666999, 0.29668769],
                   [0.52324486, 0.23841164, 0.23834346],
                   [0.5268591 , 0.23660533, 0.23653553],
                   [0.65397811, 0.17312808, 0.17289382],
                   [0.61190444, 0.19416356, 0.19393198]])
        """
        return VW.predict(self, X=X)

    def _get_vw_params(self):
        """This returns specific vw parameters to inject at fit"""
        return self.estimator_

    def _get_est_params(self):
        """This returns only the set of estimator parameters currently in use"""
        params = VWClassifier._get_est_params(self)
        params.update(dict(estimator_=self.estimator_))
        return params

    def _more_tags(self):
        return dict(binary_only=False, poor_score=True)


def tovw(x, y=None, sample_weight=None, convert_labels=False):
    """Convert array or sparse matrix to Vowpal Wabbit format

    Args:

        x : {array-like, sparse matrix}, shape (n_samples, n_features)
            Training vector, where n_samples is the number of samples and
            n_features is the number of features.
        y : {array-like}, shape (n_samples,), optional
            Target vector relative to X.
        sample_weight : {array-like}, shape (n_samples,), optional
                        sample weight vector relative to X.
        convert_labels : {bool} convert labels of the form [0,1] to [-1,1]

    Returns:
        {array-like}, shape (n_samples, 1)
            Training vectors in VW string format

    Examples:
        >>> import pandas as pd
        >>> from sklearn.feature_extraction.text import HashingVectorizer
        >>> from vowpalwabbit.sklearn import tovw
        >>> X = pd.Series(['cat', 'dog', 'cat', 'cat'], name='catdog')
        >>> y = pd.Series([-1, 1, -1, -1], name='label')
        >>> hv = HashingVectorizer()
        >>> hashed = hv.fit_transform(X)
        >>> tovw(x=hashed, y=y)
        ['-1 1 | 300839:1', '1 1 | 980517:-1', '-1 1 | 300839:1', '-1 1 | 300839:1']
    """

    use_truth = y is not None
    use_weight = sample_weight is not None

    if use_truth:
        x, y = check_X_y(x, y, accept_sparse=True)
    else:
        x = check_array(x, accept_sparse=True)

    if use_weight:
        sample_weight = check_array(
            sample_weight, accept_sparse=False, ensure_2d=False, dtype=np.int, order="C"
        )
        if sample_weight.ndim != 1:
            raise ValueError("Sample weights must be 1D array or scalar")
        if sample_weight.shape != (x.shape[0],):
            raise ValueError(
                "Sample weight shape == {}, expected {}".format(
                    sample_weight.shape, (x.shape[0],)
                )
            )
    else:
        sample_weight = np.ones(x.shape[0], dtype=np.int)

    # convert labels of the form [0,1] to [-1,1]
    if convert_labels:
        y = np.where(y < 1, -1, 1)

    rows, cols = x.shape

    # check for invalid characters if array has string values
    if x.dtype.char == "S":
        for row in rows:
            for col in cols:
                x[row, col] = INVALID_CHARS.sub(".", x[row, col])

    # convert input to svmlight format
    s = io.BytesIO()
    dump_svmlight_file(x, np.zeros(rows), s)

    # parse entries to construct VW format
    rows = s.getvalue().decode("ascii").split("\n")[:-1]
    out = []
    for idx, row in enumerate(rows):
        truth = y[idx] if use_truth else 1
        weight = sample_weight[idx]
        features = row.split("0 ", 1)[1]
        # only using a single namespace and no tags
        out.append(
            ("{y} {w} |{ns} {x}".format(y=truth, w=weight, ns=DEFAULT_NS, x=features))
        )

    s.close()

    return out

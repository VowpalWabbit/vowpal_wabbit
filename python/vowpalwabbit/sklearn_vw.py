# -*- coding: utf-8 -*-
# pylint: unused-argument, invalid-name, too-many-arguments, too-many-locals
"""Utilities to support integration of Vowpal Wabbit and scikit-learn"""

import numpy as np
import re
import io

from scipy.sparse import csr_matrix
from sklearn.base import BaseEstimator, RegressorMixin
from sklearn.linear_model.base import LinearClassifierMixin, SparseCoefMixin
from sklearn.datasets.svmlight_format import dump_svmlight_file
from sklearn.utils.validation import check_is_fitted
from sklearn.utils import shuffle
from vowpalwabbit import pyvw


DEFAULT_NS = ''
CONSTANT_HASH = 116060
INVALID_CHARS = re.compile(r"[\|: \n]+")


class VW(BaseEstimator):
    """Vowpal Wabbit Scikit-learn Base Estimator wrapper

        Attributes
        ----------
        params : {dict}
                 dictionary of model parameter keys and values
        fit_ : {bool}
               this variable is only created after the model is fitted
    """

    params = dict()

    def __init__(self,
                 rank=None,
                 lrq=None,
                 lrqdropout=None,
                 probabilities=None,
                 random_seed=None,
                 ring_size=None,
                 convert_to_vw=None,
                 bfgs=None,
                 mem=None,
                 ftrl=None,
                 ftrl_alpha=None,
                 ftrl_beta=None,
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
                 input_feature_regularizer=None,
                 audit=None,
                 a=None,
                 progress=None,
                 P=None,
                 quiet=None,
                 data=None,
                 d=None,
                 cache=None,
                 c=None,
                 k=None,
                 passes=None,
                 no_stdin=None,
                 hash=None,
                 ignore=None,
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
                 min_prediction=None,
                 max_prediction=None,
                 sort_features=None,
                 loss_function=None,
                 link=None,
                 quantile_tau=None,
                 l1=None,
                 l2=None,
                 named_labels=None,
                 final_regressor=None,
                 f=None,
                 readable_model=None,
                 invert_hash=None,
                 save_resume=None,
                 output_feature_regularizer_binary=None,
                 output_feature_regularizer_text=None,
                 oaa=None,
                 ect=None,
                 csoaa=None,
                 wap=None,
                 nn=None,
                 dropout=None,
                 inpass=None,
                 meanfield=None,
                 multitask=None):
        """VW model constructor, exposing all supported parameters to keep sklearn happy

        Parameters
        ----------
        probabilities
        random_seed (int): seed random number generator
        ring_size (int): size of example ring
        convert_to_vw (bool): flag to convert X input to vw format

        Update options
        bfgs: use L-BFGS optimization algorithm
        mem: set the rank of the inverse hessian approximation used by bfgs
        ftrl: use FTRL-Proximal optimization algorithm
        ftrl_alpha: ftrl alpha parameter
        ftrl_beta: ftrl beta parameter
        learning_rate,l (float): Set learning rate
        power_t (float): t power value
        decay_learning_rate (float): Set Decay factor for learning_rate between passes
        initial_t (float): initial t value
        feature_mask (str): Use existing regressor to determine which parameters may be updated.
                            If no initial_regressor given, also used for initial weights.

        Weight options
        initial_regressor,i (str): Initial regressor(s)
        initial_weight (float): Set all weights to an initial value of arg.
        random_weights (bool): make initial weights random
        input_feature_regularizer (str): Per feature regularization input file

        Diagnostic options
        audit,a (bool): print weights of features
        progress,P (str): Progress update frequency. int: additive, float: multiplicative
        quiet (bool): Don't output disgnostics and progress updates

        Input options
        data,d (str): path to data file for fitting external to sklearn
        cache,c (bool): use a cache. default is <data>.cache
        cache_file (str): path to cache file to use
        k (bool): auto delete cache file
        passes (int): Number of training passes

        Feature options
        hash (str): how to hash the features. Available options: strings, all
        ignore (str): ignore namespaces beginning with character <arg>
        keep (str): keep namespaces beginning with character <arg>
        redefine (str): Redefine namespaces beginning with characters of string S as namespace N. <arg> shall be in
                        form 'N:=S' where := is operator. Empty N or S are treated as default namespace.
                        Use ':' as a wildcard in S.
        bit_precision,b (int): number of bits in the feature table
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

        Example options
        testonly,t (bool): Ignore label information and just test
        min_prediction (float): Smallest prediction to output
        max_prediction (float): Largest prediction to output
        sort_features (bool): turn this on to disregard order in which features have been defined. This will lead to
                              smaller cache sizes
        loss_function (str): default_value("squared"), "Specify the loss function to be used, uses squared by default.
                             Currently available ones are squared, classic, hinge, logistic and quantile.
        link (str): apply a link function to convert output: e.g. 'logistic'
        quantile_tau (float): default_value(0.5), "Parameter \\tau associated with Quantile loss. Defaults to 0.5
        l1 (float): l_1 lambda
        l2 (float): l_2 lambda
        named_labels (str): use names for labels (multiclass, etc.) rather than integers, argument specified all
                            possible labels, comma-sep, eg \"--named_labels Noun,Verb,Adj,Punc\"

        Output model
        final_regressor,f (str): Final regressor
        readable_model (str): Output human-readable final regressor with numeric features
        invert_hash (str): Output human-readable final regressor with feature names.  Computationally expensive.
        save_resume (bool): save extra state so learning can be resumed later with new data
        output_feature_regularizer_binary (str): Per feature regularization output file
        output_feature_regularizer_text (str): Per feature regularization output file, in text

        Multiclass options
        oaa (int): Use one-against-all multiclass learning with labels
        ect (int): Use error correcting tournament multiclass learning
        csoaa (int): Use cost sensitive one-against-all multiclass learning
        wap (int): Use weighted all pairs multiclass learning

        Contextual Bandit Optimization
        cb (int): Use contextual bandit learning with specified costs
        cbify (int): Convert multiclass on <k> classes into a contextual bandit problem

        Neural Network options
        nn (int): Use a sigmoidal feed-forward neural network with N hidden units
        dropout: Train or test sigmoidal feed-forward network using dropout
        inpass: Train or test sigmoidal feed-forward network with input pass-through
        multitask: Share hidden layer across all reduced tasks
        meanfield: Train or test sigmoidal feed-forward network using mean field

        Returns
        -------
        (BaseEstimator): Returns self
        """

        # clear estimator attributes
        for attr in ['fit_', 'passes_', 'convert_to_vw_', 'vw_']:
            if hasattr(self, attr):
                delattr(self, attr)
        del attr

        # reset params and quiet models by default
        self.params = {'quiet':  True}

        # assign all valid args to params dict
        args = dict(locals())
        for k, v in args.items():
            if k != 'self' and k != '__class__' and v is not None:
                self.params[k] = v

        ext_file_args = ['data', 'd']
        if any(x in self.params for x in ext_file_args):
            # fitting will be handled by vw directly
            self.fit_ = True
            self.passes_ = 1
        else:
            # store passes separately to be used in fit
            self.passes_ = self.params.pop('passes', 1)
            if self.params.get('bfgs'):
                raise RuntimeError('An external data file must be used to fit models using the bfgs option')

        # pull out convert_to_vw from params
        self.convert_to_vw_ = self.params.pop('convert_to_vw', True)

        self.vw_ = None
        super(VW, self).__init__()

    def get_vw(self):
        """Factory to create a vw instance on demand

        Returns
        -------
        pyvw.vw instance
        """
        if self.vw_ is None:
            self.vw_ = pyvw.vw(**self.params)

        return self.vw_

    def fit(self, X, y=None, sample_weight=None):
        """Fit the model according to the given training data

        TODO: for first pass create and store example objects.
                for N-1 passes use example objects directly (simulate cache file...but in memory for faster processing)

        Parameters
        ----------
        X : {array-like, sparse matrix}, shape (n_samples, n_features or 1 if not convert_to_vw) or
            Training vector, where n_samples in the number of samples and
            n_features is the number of features.
            if not using convert_to_vw, X is expected to be a list of vw formatted feature vector strings with labels
        y : array-like, shape (n_samples,), optional if not convert_to_vw
            Target vector relative to X.
        sample_weight : array-like, shape (n_samples,)
                        sample weight vector relative to X.

        Returns
        -------
        return self so pipeline can call transform() after fit
        """
        if self.convert_to_vw_:
            X = tovw(x=X, y=y, sample_weight=sample_weight)

        model = self.get_vw()

        # add examples to model
        for n in range(self.passes_):
            if n >= 1:
                X_ = shuffle(X)
            else:
                X_ = X
            for idx, x in enumerate(X_):
                model.learn(x)
        self.fit_ = True
        return self

    def transform(self, X, y=None):
        """Transform does nothing by default besides closing the model. Transform is required for any estimator
         in a sklearn pipeline that isn't the final estimator

        Parameters
        ----------
        X : {array-like, sparse matrix}, shape (n_samples, n_features or 1 if not convert_to_vw) or
            Training vector, where n_samples in the number of samples and
            n_features is the number of features.
            if not using convert_to_vw, X is expected to be a list of vw formatted feature vector strings with labels
        y : array-like, shape (n_samples,), optional if not convert_to_vw
            Target vector relative to X.

        Returns
        -------
        return X to be passed into next estimator in pipeline
        """
        if not self.get_vw().finished:
            self.get_vw().finish()
        return X

    def predict(self, X):
        """Predict with Vowpal Wabbit model

        Parameters
        ----------
        X : {array-like, sparse matrix}, shape (n_samples, n_features or 1)
            Training vector, where n_samples in the number of samples and
            n_features is the number of features.
            if not using convert_to_vw, X is expected to be a list of vw formatted feature vector strings with labels

        Returns
        -------
        y : array-like, shape (n_samples, 1 or n_classes)
            Output vector relative to X.
        """

        check_is_fitted(self, 'fit_')

        try:
            num_samples = X.shape[0] if X.ndim > 1 else len(X)
        except AttributeError:
            num_samples = len(X)

        if self.convert_to_vw_:
            X = tovw(X)

        model = self.get_vw()

        shape = [num_samples]
        if 'oaa' in self.params and 'probabilities' in self.params:
            shape.append(self.params['oaa'])
        y = np.empty(shape)

        # predict examples
        for idx, x in enumerate(X):
            y[idx] = model.predict(ec=x)

        return y

    def __str__(self):
        if self.params is not None:
            return str(self.params)

    def __repr__(self):
        return self.__str__()

    def __del__(self):
        if hasattr(self, 'vw_') and self.vw_ is not None:
            del self.vw_

    def get_params(self, deep=True):
        """This returns the set of vw and estimator parameters currently in use"""
        out = dict()
        # add in the vw params
        out.update(self.params)
        # add in the estimator params
        out['passes'] = self.passes_
        out['convert_to_vw'] = self.convert_to_vw_
        return out

    def set_params(self, **params):
        """This destroys and recreates the Vowpal Wabbit model with updated parameters
            any parameters not provided will remain as they were initialized to at construction

        Parameters
        ----------
        params : {dict}
                 dictionary of model parameter keys and values to update
        """

        self.params.update(params)

        # manage passes and convert_to_vw params different because they are estimator params, not vw params
        if 'passes' not in params:
            self.params['passes'] = self.passes_
        if 'convert_to_vw' not in params:
            self.params['convert_to_vw'] = self.convert_to_vw_

        self.__init__(**self.params)
        return self

    def get_coefs(self):
        """Returns coefficient weights as ordered sparse matrix

        Returns
        -------
        {sparse matrix} coefficient weights for model
        """

        model = self.get_vw()
        return csr_matrix([model.get_weight(i) for i in range(model.num_weights())])

    def set_coefs(self, coefs):
        """Sets coefficients weights from ordered sparse matrix

        Parameters
        ----------
        coefs : {sparse matrix} coefficient weights for model
        """

        model = self.get_vw()
        for i in range(coefs.getnnz()):
            model.set_weight(int(coefs.indices[i]), 0, float(coefs.data[i]))

    def get_intercept(self):
        """ Returns intercept weight for model

        Returns
        -------
        {int} intercept value, 0 if noconstant
        """

        return self.get_vw().get_weight(CONSTANT_HASH)

    def save(self, filename):
        """Save model to file"""
        model = self.get_vw()
        model.save(filename)

    def load(self, filename):
        """Load model from file"""
        params = {}
        params.update(self.params)
        params["initial_regressor"] = filename
        self.set_params(**params)

        # Assume that the model is already fitted when loaded from file.
        self.fit_ = True


class ThresholdingLinearClassifierMixin(LinearClassifierMixin):
    """Mixin for linear classifiers.  A threshold is used to specify the positive
    class cutoff

    Handles prediction for sparse and dense X.
    """

    classes_ = np.array([-1., 1.])

    def __init__(self, **params):

        # assume 0 as positive score threshold
        self.pos_threshold = params.pop('pos_threshold', 0.0)

        super(ThresholdingLinearClassifierMixin, self).__init__(**params)

    def predict(self, X):
        """Predict class labels for samples in X.

        Parameters
        ----------
        X : {array-like, sparse matrix}, shape = [n_samples, n_features]
            Samples.

        Returns
        -------
        C : array, shape = [n_samples]
            Predicted class label per sample.
        """
        scores = self.decision_function(X)
        if len(scores.shape) == 1:
            indices = (scores >= self.pos_threshold).astype(np.int)
        else:
            indices = scores.argmax(axis=1)
        return self.classes_[indices]


class VWClassifier(SparseCoefMixin, ThresholdingLinearClassifierMixin, VW):
    """Vowpal Wabbit Classifier model
    Only supports binary classification currently. Use VW directly for multiclass classification
    note - don't try to apply link='logistic' on top of the existing functionality
    """

    def __init__(self, **params):

        # assume logistic loss functions
        if 'loss_function' not in params:
            params['loss_function'] = 'logistic'

        super(VWClassifier, self).__init__(**params)

    def predict(self, X):
        """Predict class labels for samples in X.

        Parameters
        ----------
        X : {array-like, sparse matrix}, shape = [n_samples, n_features]
            Samples.

        Returns
        -------
        C : array, shape = [n_samples]
            Predicted class label per sample.
        """

        return ThresholdingLinearClassifierMixin.predict(self, X=X)

    def decision_function(self, X):
        """Predict confidence scores for samples.
        The confidence score for a sample is the signed distance of that
        sample to the hyperplane.

        Parameters
        ----------
        X : {array-like, sparse matrix}, shape = (n_samples, n_features)
            Samples.

        Returns
        -------
        array, shape=(n_samples,) if n_classes == 2 else (n_samples, n_classes)
            Confidence scores per (sample, class) combination. In the binary
            case, confidence score for self.classes_[1] where >0 means this
            class would be predicted.
        """

        return VW.predict(self, X=X)


class VWRegressor(VW, RegressorMixin):
    """Vowpal Wabbit Regressor model """

    pass


def tovw(x, y=None, sample_weight=None):
    """Convert array or sparse matrix to Vowpal Wabbit format

    Parameters
    ----------
    x : {array-like, sparse matrix}, shape (n_samples, n_features)
        Training vector, where n_samples is the number of samples and
        n_features is the number of features.
    y : {array-like}, shape (n_samples,), optional
        Target vector relative to X.
    sample_weight : {array-like}, shape (n_samples,), optional
                    sample weight vector relative to X.

    Returns
    -------
    out : {array-like}, shape (n_samples, 1)
          Training vectors in VW string format

    Examples
    --------
    >>> import pandas as pd
    >>> from sklearn.feature_extraction.text import HashingVectorizer
    >>> from vowpalwabbit.sklearn_vw import tovw
    >>> X = pd.Series(['cat', 'dog', 'cat', 'cat'], name='catdog')
    >>> y = pd.Series([-1, 1, -1, -1], name='label')
    >>> hv = HashingVectorizer()
    >>> hashed = hv.fit_transform(X)
    >>> tovw(x=hashed, y=y)
    """

    use_truth = y is not None
    use_weight = sample_weight is not None

    # convert to numpy array if needed
    if not isinstance(x, (np.ndarray, csr_matrix)):
        x = np.array(x)
    if not isinstance(y, np.ndarray):
        y = np.array(y)

    # make sure this is a 2d array
    if x.ndim == 1:
        x = x.reshape(1, -1)
    if y.ndim == 0:
        y = y.reshape(1)

    rows, cols = x.shape

    # check for invalid characters if array has string values
    if x.dtype.char == 'S':
        for row in rows:
            for col in cols:
                x[row, col] = INVALID_CHARS.sub('.', x[row, col])

    # convert input to svmlight format
    s = io.BytesIO()
    dump_svmlight_file(x, np.zeros(rows), s)

    # parse entries to construct VW format
    rows = s.getvalue().decode('ascii').split('\n')[:-1]
    out = []
    for idx, row in enumerate(rows):
        truth = y[idx] if use_truth else 1
        weight = sample_weight[idx] if use_weight else 1
        features = row.split('0 ', 1)[1]
        # only using a single namespace and no tags
        out.append(('{y} {w} |{ns} {x}'.format(y=truth, w=weight, ns=DEFAULT_NS, x=features)))

    s.close()

    return out

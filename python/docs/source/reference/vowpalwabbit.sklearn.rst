
vowpalwabbit.sklearn
====================

This is an optional module which implements `sklearn` compatability.

Deprecated alias
----------------

`vowpalwabbit.sklearn_vw` is a deprecated alias for :py:obj:`vowpalwabbit.sklearn`

Example usage
-------------

.. code-block:: python

    import numpy as np
    from sklearn import datasets
    from sklearn.model_selection import train_test_split
    from vowpalwabbit.sklearn import VWClassifier
        # generate some data
    X, y = datasets.make_hastie_10_2(n_samples=10000, random_state=1)
    X = X.astype(np.float32)
        # split train and test set
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=256)
        # build model
    model = VWClassifier()
    model.fit(X_train, y_train)
        # predict model
    y_pred = model.predict(X_test)
        # evaluate model
    model.score(X_train, y_train)
    model.score(X_test, y_test)

Module contents
---------------

.. automodule:: vowpalwabbit.sklearn
    :members:
    :undoc-members:
    :show-inheritance:

Vowpal Wabbit Python Wrapper
============================

Vowpal Wabbit is a fast machine learning library for online learning.

Installation
------------

From PyPI:

.. code-block:: bash

    $ pip install vowpalwabbit

From remote repo:

.. code-block:: bash

    $ pip install -e git+https://github.com/JohnLangford/vowpal_wabbit/python

From local repo:

.. code-block:: bash

    $ cd python
    $ python setup.py install

or

.. code-block::

    $ cd python
    $ pip install -e .

Usage
-----

You can use the python wrapper directly like this:

.. code-block:: python

    >>> from vowpalwabbit impor pyvw
    >>> vw = pyvw.vw(quiet=True)
    >>> ex = vw.example('1 | a b c')
    >>> vw.learn(ex)
    >>> vw.predict(ex)

Or you can use the scikit-learn interface like this:

.. code-block:: python

    >>> import numpy as np
    >>> from sklearn import datasets
    >>> from sklearn.cross_validation import train_test_split
    >>> from vowpalwabbit.sklearn_vw import VWClassifier
    >>>
    >>> # generate some data
    >>> X, y = datasets.make_hastie_10_2(n_samples=10000, random_state=1)
    >>> X = X.astype(np.float32)
    >>>
    >>> # split train and test set
    >>> X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=256)
    >>>
    >>> # build model
    >>> model = VWClassifier()
    >>> model.fit(X_train, y_train)
    >>>
    >>> # evaluate model
    >>> model.score(X_train, y_train)
    >>> model.score(X_test, y_test))

Development
-----------

Contributions are welcome for improving the python wrapper to Vowpal Wabbit.

1. Check for open issues_ or create one to discuss a feature idea or bug.
2. Fork the repo_ on Github and make changes to the master branch (or a new branch off of master).
3. Write a test in the python/tests folder showing the bug was fixed or feature works (recommend using pytest_).
4. Make sure package installs and tests pass under all supported environments (this calls tox_ automatically).
5. Send the pull request.

Tests can be run using setup.py:

.. code-block:: bash

    $ python setup.py test


Directory Structure:

* python : this is where the c++ extension lives
* python/vowpalwabbit : this is then main directory for python wrapper code and utilities
* python/examples : example python code and jupyter notebooks to demonstrate functionality
* python/tests : contains all tests for python code

**Note:** neither examples nor tests directories are included in the distributed package, they are only for development purposes.

.. _issues: https://github.com/JohnLangford/vowpal_wabbit/issues
.. _repo: https://github.com/JohnLangford/vowpal_wabbit
.. _pytest: http://pytest.org/latest/getting-started.html
.. _tox: https://tox.readthedocs.io/en/latest/index.html



Vowpal Wabbit Python Wrapper
============================

|ImageLink|_

.. |ImageLink| image:: https://badge.fury.io/py/vowpalwabbit.svg
.. _ImageLink: https://pypi.python.org/pypi/vowpalwabbit

Vowpal Wabbit is a fast machine learning library for online learning, and this is the python wrapper for the project.

Requirements
------------

**Python Version:** 3.10, 3.11, 3.12, or 3.13

Pre-built wheels are available on PyPI for:

* Linux (x86_64, aarch64)
* macOS (x86_64, arm64)
* Windows (x86_64)

Installing this package builds Vowpal Wabbit locally for explicit use within python, it will not create the command-line version
of the tool (or affect any previously existing command-line installations).
To install the command-line version see the main project page: https://github.com/VowpalWabbit/vowpal_wabbit

The version of the PyPI vowpalwabbit package corresponds to the tagged version of the code in the github repo that will be used
during building and installation.
If you need to make local changes to the code and rebuild the python binding be sure to pip uninstall vowpalwabbit then rebuild
using the local repo installation instructions below.

Installation
------------

See the `installation instructions`_


Usage
-----

You can use the python wrapper directly like this:

.. code-block:: python

    >>> import vowpalwabbit
    >>> vw = vowpalwabbit.Workspace(quiet=True)
    >>> ex = vw.example('1 | a b c')
    >>> vw.learn(ex)
    >>> vw.predict(ex)

Or you can use the included scikit-learn interface like this:

.. code-block:: python

    >>> import numpy as np
    >>> from sklearn import datasets
    >>> from sklearn.model_selection import train_test_split
    >>> from vowpalwabbit.sklearn import VWClassifier
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
    >>> # predict model
    >>> y_pred = model.predict(X_test)
    >>>
    >>> # evaluate model
    >>> model.score(X_train, y_train)
    >>> model.score(X_test, y_test)

Troubleshooting
---------------

See the `troubleshooting guide`_

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

.. _issues: https://github.com/VowpalWabbit/vowpal_wabbit/issues
.. _repo: https://github.com/VowpalWabbit/vowpal_wabbit
.. _pytest: http://pytest.org/latest/getting-started.html
.. _tox: https://tox.readthedocs.io/en/latest/index.html
.. _installation instructions: https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Python#installing
.. _troubleshooting guide: https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Python#troubleshooting

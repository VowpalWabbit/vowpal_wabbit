Vowpal Wabbit Python Wrapper
============================

.. image:: https://badge.fury.io/py/vowpalwabbit.svg
    :alt: PyPI Package
    :target: https://pypi.python.org/pypi/vowpalwabbit
.. image:: https://travis-ci.org/VowpalWabbit/vowpal_wabbit.svg?branch=master
    :alt: Build Status
    :target: https://travis-ci.org/VowpalWabbit/vowpal_wabbit
.. image:: https://ci.appveyor.com/api/projects/status/6hqpd9e64h72gybr/branch/master?svg=true
    :alt: Windows Build Status
    :target: https://ci.appveyor.com/project/VowpalWabbit/vowpal-wabbit
.. image:: https://coveralls.io/repos/github/VowpalWabbit/vowpal_wabbit/badge.svg
    :alt: Coverage
    :target: https://coveralls.io/r/VowpalWabbit/vowpal_wabbit

Vowpal Wabbit is a fast machine learning library for online learning, and this is the python wrapper for the project.

Installing this package builds Vowpal Wabbit locally for explicit use within python, it will not create the command-line version
of the tool (or affect any previously existing command-line installations).
To install the command-line version see the main project page: https://github.com/VowpalWabbit/vowpal_wabbit

The version of the PyPI vowpalwabbit package corresponds to the tagged version of the code in the github repo that will be used
during building and installation.
If you need to make local changes to the code and rebuild the python binding be sure to pip uninstall vowpalwabbit then rebuild
using the local repo installation instructions below.

Installation
------------

From PyPI:

Linux/Mac OSX:

.. code-block:: bash

    $ pip install vowpalwabbit

Windows:

.. code-block:: bat

    > pip install --global-option="--vcpkg-root=path\to\vcpkg" vowpalwabbit
    


From local repo (useful when making modifications):

.. code-block:: bash

    # Dependencies
    $ sudo apt install libboost-dev libboost-program-options-dev libboost-system-dev libboost-thread-dev libboost-math-dev libboost-test-dev libboost-python-dev zlib1g-dev cmake 
    
    # Build and install package
    $ python setup.py install


Usage
-----

You can use the python wrapper directly like this:

.. code-block:: python

    >>> from vowpalwabbit import pyvw
    >>> vw = pyvw.vw(quiet=True)
    >>> ex = vw.example('1 | a b c')
    >>> vw.learn(ex)
    >>> vw.predict(ex)

Or you can use the included scikit-learn interface like this:

.. code-block:: python

    >>> import numpy as np
    >>> from sklearn import datasets
    >>> from sklearn.model_selection import train_test_split
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
    >>> # predict model
    >>> y_pred = model.predict(X_test)
    >>>
    >>> # evaluate model
    >>> model.score(X_train, y_train)
    >>> model.score(X_test, y_test)

Troubleshooting
---------------

Some common causes of failure for installation are due to missing or mis-matched dependencies when Vowpal Wabbit builds.
Make sure you have boost and boost-python installed on your system.

For Ubuntu/Debian/Mint

.. code-block:: bash

    $ apt install libboost-dev libboost-program-options-dev libboost-system-dev libboost-thread-dev libboost-math-dev libboost-test-dev libboost-python-dev zlib1g-dev cmake 

For Mac OSX

.. code-block:: bash

    $ brew install cmake
    $ brew install boost
    #If you want to build with python 2 support
    brew install boost-python
    #If you want to build with python 3 support
    brew install boost-python3
    
For Windows

    1. Install vcpkg_

    2. Run

.. code-block:: bat

    > vcpkg --triplet x64-windows install zlib boost-system boost-program-options boost-test boost-align boost-foreach boost-python boost-math boost-thread python3 boost-python

.. _vcpkg: https://github.com/microsoft/vcpkg

Installing Vowpal Wabbit under an Anaconda environment (on OSX or Linux) can be done using the following steps:

.. code-block:: bash

    $ git clone https://github.com/VowpalWabbit/vowpal_wabbit.git
    # create conda environment if necessary
    $ conda create -n vowpalwabbit
    $ source activate vowpalwabbit
    # install necessary boost dependencies
    $ conda install -y -c anaconda boost
    $ pip install -e vowpal_wabbit
    
**For python3 on Ubuntu 16.04 LTS**: Ubuntu 16.04 defaults to an old, custom-built version of boost. As such, the boost_python library names do not follow the standard naming convention adopted by offical boost releases for the boost_python libraries.
You may need to manually create the relevant symlinks in this case. Example commands for python 3.5 follows: 

.. code-block:: bash

    $ cd /usr/lib/x86_64-linux-gnu/
    $ sudo ln -s libboost_python-py35.so libboost_python3.so
    $ sudo ln -s libboost_python-py35.a libboost_python3.a

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

Experimental build for Windows
------------------------------

An extension on the `experimental Windows CMake build`_ for the main project.

**Note:** attempting to install boost-python in vcpkg while multiple python versions are installed in vcpkg will cause errors. Ensure only the relevant python version is installed in the environment before proceeding.

Python3
~~~~~~~

1. install required vcpkgs

.. code-block:: bat

    > vcpkg install python3:x64-windows
    > vcpkg install boost-python:x64-windows
    
2. Run

.. code-block:: bat

    > python setup.py --vcpkg-root=path\to\vcpkg install

Python2
~~~~~~~

Due to limitations in the current version of boost-python, some manual changes must be made to the vcpkg tools

1. Edit [vcpkg-root]\\ports\\boost-python
2. Edit the file CONTROL
    a. Change the Build-Depends entry for **python3** to **python2**
3. install required vcpkgs

.. code-block:: bat

    > vcpkg install python2:x64-windows
    > vcpkg install boost-python:x64-windows

4. Run

.. code-block:: bat

    > python setup.py --vcpkg-root=path\to\vcpkg install
    
.. _experimental Windows CMake build: https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Building#experimental-using-cmake-on-windows

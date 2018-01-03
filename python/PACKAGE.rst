Python Packaging
================

Setting up the python package for distribution on PyPI is done using commands through setup.py
The following instructions assume you are working from the python directory.

Deployment Process
------------------

0) Commit changes (increment the version in configure.ac file, PyPI will not overwrite a package using the same version)
1) Update MANIFEST.in to include any additional files then check it to make sure the dist has the right data in it

    .. code-block:: bash

        $ check-manifest --ignore Makefile,PACKAGE.rst,*.cc,tox.ini,tests*,examples*,src*

2) Lint the code:

   .. code-block:: bash

       $ pylint -f colorized vowpalwabbit

3) Make sure code passes all tests under supported environments

    .. code-block:: bash

        # install virtualenv if necessary
        $ pip install virtualenv
        $ python setup.py test


4) Create dist folder for package

    .. code-block:: bash

        $ python setup.py sdist

5) Upload package to PyPI

    You should have twine installed and configured and your PyPI / test PyPI user should have access to the package
    <VERSION> corresponds to the new version in the configure.ac file which should match what step 4) creates in the dist sub-dir

    a) Test package

    .. code-block:: bash

        $ twine upload --repository-url https://test.pypi.org/legacy/ -u vowpalwabbit dist/vowpalwabbit-<VERSION>.tar.gz
        $ cd /tmp
        $ virtualenv test_vw_package
        $ source test_vw_package/bin/activate
        $ pip install -i https://testpypi.python.org/simple/ vowpalwabbit
        $ python -c 'from vowpalwabbit import pyvw'
        $ deactivate
        $ rm -rf test_vw_package

    b) Upload package

    .. code-block:: bash

        $ twine upload -u <username> -p <password> dist/vowpalwabbit-<VERSION>.tar.gz

6) Cleanup build and packaging artifacts / directories

    .. code-block:: bash

        $ python setup.py clean

References
==========

https://packaging.python.org/en/latest/distributing/

http://setuptools.readthedocs.io/en/latest/setuptools.html

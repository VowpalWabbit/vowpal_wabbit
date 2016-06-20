Python Packaging
================

Setting up the python package for distribution on PyPI is done using commands through setup.py
The following instructions assume you are working from the python directory.

Deployment Process
------------------

0) push changes (increment the version in configure.ac file, PyPI will not overwrite a package using the same version)
1) Make sure code passes all tests under supported environments

    .. code-block:: bash

        $ python setup.py test

2) Create dist folder for package

    .. code-block:: bash

        $ python setup.py sdist

3) Upload package to PyPI

    You should have twine installed and configured and your PyPI / test PyPI user should have access to the package
    <VERSION> corresponds to the new version in the configure.ac file

    a) Test package

    .. code-block:: bash

        $ twine upload -r test dist/*
        $ virtualenv test_vw_package
        $ source test_vw_package/bin/activate
        $ pip install -i https://testpypi.python.org/simple/ vowpalwabbit
        $ deactivate
        $ rm -rf test_vw_package

    b) Upload package

    .. code-block:: bash

        $ twine upload dist dist/vowpalwabbit-<VERSION>.tar.gz

4) Cleanup build and packaging artifacts / directories

    .. code-block:: bash

        $ python setup.py clean

References
==========

https://packaging.python.org/en/latest/distributing/

http://setuptools.readthedocs.io/en/latest/setuptools.html

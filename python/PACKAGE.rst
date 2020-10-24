Python Packaging
================

Binary wheels are produced in CI according to the `support table`_ and uploaded to PyPi for a release.

Deployment Process
------------------

0) Binary wheels are automatically produced for every commit to master and uploaded as artifacts in the CI
    * `Linux`_
    * `Windows`_
    * `MacOS`_
1) When it is time to release a version on PyPi, download the set of artifacts from each workflow for the commit to release
2) Upload package to PyPI

    You should have twine installed and configured and your PyPI / test PyPI user should have access to the package
    <VERSION> corresponds to the new version in the `version.txt` file.

    a) Test package

    .. code-block:: bash
        
        $ # Repeat for each wheel
        $ twine upload --repository-url https://test.pypi.org/legacy/ -u vowpalwabbit vowpalwabbit-<VERSION>-cp37-cp37m-manylinux2010_x86_64.whl
        $ cd /tmp
        $ virtualenv test_vw_package
        $ source test_vw_package/bin/activate
        $ pip install -i https://testpypi.python.org/simple/ vowpalwabbit
        $ python -c 'from vowpalwabbit import pyvw'
        $ deactivate
        $ rm -rf test_vw_package

    b) Upload package

    .. code-block:: bash
    
        $ # Repeat for each wheel
        $ twine upload -u <username> -p <password>  vowpalwabbit-<VERSION>-cp37-cp37m-manylinux2010_x86_64.whl

.. _support table: https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Python#support
.. _Linux: https://github.com/VowpalWabbit/vowpal_wabbit/actions?query=workflow%3A%22Build+Linux+Python+Wheels%22
.. _Windows: https://github.com/VowpalWabbit/vowpal_wabbit/actions?query=workflow%3A%22Build+Windows+Python+Wheels%22
.. _MacOS: https://github.com/VowpalWabbit/vowpal_wabbit/actions?query=workflow%3A%22Build+MacOS+Python+Wheels%22

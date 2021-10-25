Python Packaging
================

| Binary wheels are produced in CI according to the `support table`_ and uploaded to PyPI for a release.
| The source is also used to by the Conda-Forge Feedstock CI to update a release on the conda-forge channel of Anaconda.

PyPI Deployment Process
-----------------------
0) Binary wheels are automatically produced for every commit to master and uploaded as artifacts in the CI
    * `Linux`_
    * `Windows`_
    * `MacOS`_
1) When it is time to release a version on PyPI, download the set of artifacts from each workflow for the commit to release
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
    
    c) Upload the source package
   
    .. code-block:: bash

        $ # Make sure you are on a clean checkout of the release branch
        $ python setup.py sdist
        $ twine upload -u <username> -p <password> dist/vowpalwabbit-<VERSION>.tar.gz

Conda Deployment Process
------------------------
See `conda-forge documentation`_ for more details on maintaining packages. The steps below should be sufficient to update the release.

0. Fork the `vowpalwabbit-feedstock repo`_
1. Update the recipe/meta.yaml file

  1. Change {% set version = "x.xx.x" %} line to desired version number 
  2. If changing the version reset build number to 0, otherwise increment the build number
  3. Pull tarballs from github for the release tag of vowpal_wabbit as well as matching commits from all submodules in ext_libs folder
  4. Compute sha256 hashes for each tarball and update urls and hashes in the source section as needed

2. Make a PR back to original vowpalwabbit-feedstock repo
3. Wait for CI tests to pass and merge

.. _support table: https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Python#support
.. _Linux: https://github.com/VowpalWabbit/vowpal_wabbit/actions?query=workflow%3A%22Build+Linux+Python+Wheels%22
.. _Windows: https://github.com/VowpalWabbit/vowpal_wabbit/actions?query=workflow%3A%22Build+Windows+Python+Wheels%22
.. _MacOS: https://github.com/VowpalWabbit/vowpal_wabbit/actions?query=workflow%3A%22Build+MacOS+Python+Wheels%22
.. _vowpalwabbit-feedstock repo: https://github.com/conda-forge/vowpalwabbit-feedstock
.. _conda-forge documentation: https://conda-forge.org/docs/index.html
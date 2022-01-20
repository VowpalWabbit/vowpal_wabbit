vowpalwabbit
============

The core functionality of the package is available in this root module. A small number of advanced usage classes are only available in :py:obj:`vowpalwabbit.pyvw`.

Example usage
-------------

.. code-block:: python

    from vowpalwabbit import Workspace, Example
    workspace = Workspace(quiet=True)
    ex = Example('1 | a b c')
    workspace.learn(ex)
    workspace.predict(ex)

Module contents
---------------

.. automodule:: vowpalwabbit
    :members:
    :undoc-members:
    :show-inheritance:

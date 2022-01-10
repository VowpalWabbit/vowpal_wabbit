# -*- coding: utf-8 -*-
"""Python interfaces for VW"""

import importlib

__all__ = ["pyvw", "sklearn_vw", "DFtoVW"]


def __getattr__(name):
    if name == "__version__":
        from .version import __version__

        return __version__
    if name in __all__:
        return importlib.import_module("." + name, __name__)
    raise AttributeError(f"module {__name__!r} has no attribute {name!r}")

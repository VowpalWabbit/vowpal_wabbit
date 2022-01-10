# -*- coding: utf-8 -*-
"""Python interfaces for VW"""

import importlib


class LazyLoader:
    def __init__(self, lib_name):
        self.lib_name = lib_name
        self._mod = None

    def __getattrib__(self, name):
        if self._mod is None:
            self._mod = importlib.import_module(self.lib_name)

        return getattr(self._mod, name)


from .version import __version__

from . import pyvw

# sklearn interface is an optional module. Lazy load it only when requested.
sklearn_vw = LazyLoader("sklearn_vw")

# Pandas converter is an optional module. Lazy load it only when requested.
DFtoVW = LazyLoader("DFtoVW")

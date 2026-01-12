# -*- coding: utf-8 -*-
"""Python interfaces for VW"""

import importlib as _importlib
import warnings as _warnings
import sys as _sys
import os as _os

# On Windows with Python 3.8+, explicitly load boost_python DLL
# Python 3.8+ changed DLL loading - we need to preload the boost DLL
# so that pylibvw.pyd can find it
if _sys.platform == 'win32' and _sys.version_info >= (3, 8):
    import ctypes as _ctypes
    import glob as _glob

    pkg_dir = _os.path.dirname(__file__)
    parent_dir = _os.path.dirname(pkg_dir)

    # Try to find and preload boost_python DLL from both locations
    for search_dir in [pkg_dir, parent_dir]:
        boost_dlls = _glob.glob(_os.path.join(search_dir, 'boost_python*.dll'))
        for dll_path in boost_dlls:
            try:
                # Preload the DLL into the process so pylibvw.pyd can find it
                _ctypes.CDLL(dll_path)
                break
            except (OSError, FileNotFoundError):
                continue

__all__ = [
    "AbstractLabel",
    "ActionScore",
    "CBContinuousLabel",
    "CBContinuousLabelElement",
    "CBEvalLabel",
    "CBLabel",
    "CBLabelElement",
    "CCBLabel",
    "CCBLabelType",
    "CCBSlotOutcome",
    "CostSensitiveElement",
    "CostSensitiveLabel",
    "Example",
    "ExampleNamespace",
    "LabelType",
    "MulticlassLabel",
    "MulticlassProbabilitiesLabel",
    "MultilabelLabel",
    "NamespaceId",
    "PredictionType",
    "SimpleLabel",
    "SlatesLabel",
    "SlatesLabelType",
    "Workspace",
]

from .version import __version__
from . import pyvw
from .pyvw import (
    AbstractLabel,
    ActionScore,
    CBContinuousLabel,
    CBContinuousLabelElement,
    CBEvalLabel,
    CBLabel,
    CBLabelElement,
    CCBLabel,
    CCBLabelType,
    CCBSlotOutcome,
    CostSensitiveElement,
    CostSensitiveLabel,
    Example,
    ExampleNamespace,
    LabelType,
    MulticlassLabel,
    MulticlassProbabilitiesLabel,
    MultilabelLabel,
    NamespaceId,
    PredictionType,
    SimpleLabel,
    SlatesLabel,
    SlatesLabelType,
    Workspace,
    merge_models,
)


def __getattr__(name):
    if name == "DFtoVW":
        name = "dftovw"
        _warnings.warn(
            "Module DFtoVW has been renamed to dftovw. Please use the new name. The old alias will be removed in a future version.",
            DeprecationWarning,
        )
        return _importlib.import_module("." + name, __name__)
    if name in __all__:
        return _importlib.import_module("." + name, __name__)
    raise AttributeError(f"module {__name__!r} has no attribute {name!r}")

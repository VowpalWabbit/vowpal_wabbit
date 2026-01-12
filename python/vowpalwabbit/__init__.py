# -*- coding: utf-8 -*-
"""Python interfaces for VW"""

import importlib as _importlib
import warnings as _warnings
import sys as _sys
import os as _os

# On Windows with Python 3.8+, add DLL directory for boost_python DLL
# Python 3.8+ no longer searches current directory for DLLs by default
if _sys.platform == 'win32' and _sys.version_info >= (3, 8):
    # Add both the package directory and site-packages root to DLL search path
    # so pylibvw.pyd can find boost_python310.dll
    pkg_dir = _os.path.dirname(__file__)
    _os.add_dll_directory(pkg_dir)
    # Also add parent directory (site-packages root) where pylibvw.pyd and the DLL are
    _os.add_dll_directory(_os.path.dirname(pkg_dir))

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

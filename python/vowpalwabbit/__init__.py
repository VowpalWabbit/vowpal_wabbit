# -*- coding: utf-8 -*-
"""Python interfaces for VW"""

import importlib as _importlib
import warnings as _warnings
import sys as _sys
import os as _os

# On Windows with Python 3.8+, explicitly load DLLs and add to search path
# Python 3.8+ changed DLL loading - need both add_dll_directory and preload
if _sys.platform == 'win32' and _sys.version_info >= (3, 8):
    import ctypes as _ctypes
    import glob as _glob

    pkg_dir = _os.path.dirname(__file__)
    parent_dir = _os.path.dirname(pkg_dir)

    # Add directories to DLL search path
    _os.add_dll_directory(parent_dir)
    _os.add_dll_directory(pkg_dir)
    if _os.environ.get('VW_DEBUG_DLL_LOAD'):
        print(f"vowpalwabbit: Added DLL directories: {parent_dir}, {pkg_dir}")

    # Also preload all DLL dependencies from both locations
    # Need to preload boost_python, zlib, and any other dependencies
    for search_dir in [parent_dir, pkg_dir]:  # Check parent first (where .pyd is)
        # Find all DLLs (*.dll) but skip python DLLs
        all_dlls = _glob.glob(_os.path.join(search_dir, '*.dll'))
        for dll_path in all_dlls:
            basename = _os.path.basename(dll_path)
            # Skip python DLLs
            if basename.startswith('python'):
                continue
            try:
                # Preload the DLL into the process so pylibvw.pyd can find it
                _ctypes.CDLL(dll_path)
                if _os.environ.get('VW_DEBUG_DLL_LOAD'):
                    print(f"vowpalwabbit: Successfully preloaded {dll_path}")
            except (OSError, FileNotFoundError) as e:
                if _os.environ.get('VW_DEBUG_DLL_LOAD'):
                    print(f"vowpalwabbit: Failed to preload {dll_path}: {e}")

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

from .version import __version__, __git_commit__
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

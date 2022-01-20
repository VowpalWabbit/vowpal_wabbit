# -*- coding: utf-8 -*-
"""Python interfaces for VW"""

import importlib as _importlib
import warnings as _warnings

__all__ = [
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
    ActionScore,
    CBContinuousLabel,
    CBContinuousLabelElement,
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
    NamespaceId,
    PredictionType,
    SimpleLabel,
    SlatesLabel,
    SlatesLabelType,
    Workspace,
)


def __getattr__(name):
    if name == "DFtoVW":
        name = "dftovw"
        _warnings.warn(
            "Module DFtoVW has been renamed to dftovw. Please use the new name. The old alias will be removed in a future version.",
            DeprecationWarning,
        )
        return _importlib.import_module("." + name, __name__)
    if name == "sklearn_vw":
        name = "sklearn"
        _warnings.warn(
            "Module sklearn_vw has been renamed to sklearn. Please use the new name. The old alias will be removed in a future version.",
            DeprecationWarning,
        )
        return _importlib.import_module("." + name, __name__)
    if name in __all__:
        return _importlib.import_module("." + name, __name__)
    raise AttributeError(f"module {__name__!r} has no attribute {name!r}")

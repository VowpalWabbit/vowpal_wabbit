# -*- coding: utf-8 -*-

import warnings as _warnings

_warnings.warn(
    "Module sklearn_vw has been renamed to sklearn. Please use the new name. The old alias will be removed in a future version.",
    DeprecationWarning,
)

from .sklearn import *

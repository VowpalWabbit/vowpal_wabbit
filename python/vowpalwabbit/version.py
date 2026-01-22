# Provides the present version of VowpalWabbit

from importlib.metadata import version

__version__ = version("vowpalwabbit")

# Git commit hash from the native library
try:
    from . import pyvw
    __git_commit__ = pyvw.__git_commit__
except (ImportError, AttributeError):
    __git_commit__ = "unknown"

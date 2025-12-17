# Provides the present version of VowpalWabbit

try:
    from importlib import metadata as importlib_metadata
except ImportError:
    import importlib_metadata

__version__ = importlib_metadata.version("vowpalwabbit")

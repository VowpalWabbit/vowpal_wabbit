# Provides the present version of VowpalWabbit

import pkg_resources

__version__ = pkg_resources.require("vowpalwabbit")[0].version

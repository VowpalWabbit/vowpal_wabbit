"""Namespaced module "vw" that just imports functionality from the
vowpalwabbit module. This module just enables running via a Python module
command,

$ python -m vw [opts]

"""

import sys

from vowpalwabbit import pyvw


def main():
    opts = sys.argv[1:]
    raise SystemExit(pyvw.vw(" ".join(opts)))

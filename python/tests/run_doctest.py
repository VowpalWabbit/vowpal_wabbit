import doctest
import sys

from vowpalwabbit import pyvw, sklearn_vw, DFtoVW

if __name__ == '__main__':
    num_failures = 0
    num_failures += doctest.testmod(pyvw)[0]
    num_failures += doctest.testmod(sklearn_vw)[0]
    num_failures += doctest.testmod(DFtoVW)[0]
    sys.exit(num_failures)
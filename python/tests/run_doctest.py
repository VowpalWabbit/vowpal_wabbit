import doctest
import sys

from vowpalwabbit import pyvw, sklearn, dftovw

if __name__ == "__main__":
    num_failures = 0
    num_failures += doctest.testmod(pyvw)[0]
    num_failures += doctest.testmod(sklearn)[0]
    num_failures += doctest.testmod(dftovw)[0]
    sys.exit(num_failures)

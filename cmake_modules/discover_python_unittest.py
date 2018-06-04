import unittest
import sys

def list_tests_from(path):
	loader = unittest.TestLoader()
	suite = loader.discover(path)
	for t1 in suite:
		tests = t1._tests
		if len(tests):
			for t1 in tests:
				for t2 in t1._tests:
					tn = t2.__str__().split()
					print(path + "." + tn[1][1:-1] + "." + tn[0])

if __name__ == "__main__":
	for arg in sys.argv[1:]:
		list_tests_from(arg)

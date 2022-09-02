from confidence_sequence import confidence_sequence
import numpy as np


def test_confidence_sequence():
    rs = [0.5, 0.6, 0.7, 0.8] * 1000
    cs = confidence_sequence()
    for r, w in zip(rs, rs):
        cs.addobs(w, r)
    alpha = 0.05
    lb, ub = cs.getci(alpha)

    # Compare bounds to confidence_sequence_test.cc
    np.testing.assert_almost_equal(lb, 0.4215480, 5)
    np.testing.assert_almost_equal(ub, 0.7907692, 5)

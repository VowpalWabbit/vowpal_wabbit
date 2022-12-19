from confidence_sequence_robust import DDRM as confidence_sequence_robust
import numpy as np


def test_confidence_sequence_robust():
    csr = confidence_sequence_robust()
    for _ in range(200):
        csr.addobs((1.1, 1))
        csr.addobs((1.1, 0))
    alpha = 0.05
    lb, ub = csr.getci(alpha)

    # Compare bounds to confidence_sequence_robust_test.cc
    np.testing.assert_almost_equal(lb, 0.4574146652500113, 6)
    np.testing.assert_almost_equal(ub, 0.5423597665906932, 6)

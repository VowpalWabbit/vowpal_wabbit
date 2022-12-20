from confidence_sequence_robust import DDRM as confidence_sequence_robust
import numpy as np
import scipy.optimize as so


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

    # Test brentq minimizer separately
    s = 219.99999999999926
    thres = 3.6888794541139363
    memo = {0: 12.680412758057498, 1: 4.354160995282192}
    minmu = 0.0
    maxmu = 1.0

    res = so.root_scalar(
        f=lambda mu: csr.upper.logwealthmix(mu=mu, s=s, thres=thres, memo=memo) - thres,
        method="brentq",
        bracket=[minmu, maxmu],
    )

    # Compare to brentq in confidence_sequence_robust_test.cc
    np.testing.assert_almost_equal(res.root, 0.4576402334093068, 6)

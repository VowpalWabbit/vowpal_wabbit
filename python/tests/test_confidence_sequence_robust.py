from confidence_sequence_robust import DDRM as confidence_sequence_robust
import numpy as np
import scipy.optimize as so


def test_cs_robust_ci():
    csr = confidence_sequence_robust()
    csr.addobs((0.265260433, 0.4))
    csr.addobs((0.210534972, 0))
    csr.addobs((0.183917071, 1))
    csr.addobs((0.559713528, 1))
    csr.addobs((0.595640365, 1))
    csr.addobs((7.180584551, 0.9))
    csr.addobs((0.138659127, 0.9))
    csr.addobs((0.663108867, 0))
    csr.addobs((0.678420014, 0.1))
    csr.addobs((0.123112832, 0.9))
    csr.addobs((0.703156298, 1))
    csr.addobs((0.713376195, 0.1))
    csr.addobs((0.722519043, 1))
    csr.addobs((0.730768415, 0.6))
    csr.addobs((0.107544107, 1))
    csr.addobs((0.745132612, 1))
    csr.addobs((0.751442272, 0.9))
    csr.addobs((4.986560376, 0.4))
    csr.addobs((0.099391065, 1))
    csr.addobs((37.85031381, 0.9))
    csr.addobs((0.096127937, 0.9))
    csr.addobs((0.094648809, 0.6))
    csr.addobs((0.78106104, 0.9))
    csr.addobs((0.784993688, 0.9))
    csr.addobs((0.090697586, 0.6))
    csr.addobs((0.792238169, 0.4))
    csr.addobs((0.088398536, 1))
    csr.addobs((0.798782221, 0.6))
    csr.addobs((0.801821345, 0.3))
    csr.addobs((0.804722505, 0.9))
    csr.addobs((0.807496314, 0.9))
    csr.addobs((0.810154443, 0.1))
    csr.addobs((0.082674622, 1))
    csr.addobs((0.815150197, 0.6))
    csr.addobs((0.081068034, 0.4))
    csr.addobs((0.080309513, 0.1))
    csr.addobs((0.079578481, 0.4))
    csr.addobs((0.078873344, 1))
    csr.addobs((0.82609249, 1))
    csr.addobs((0.828055494, 0.7))
    csr.addobs((0.076898241, 1))
    csr.addobs((0.076283066, 1))
    csr.addobs((0.075686285, 1))
    csr.addobs((0.075107653, 0.4))
    csr.addobs((0.074546428, 1))
    csr.addobs((0.074001442, 1))
    csr.addobs((0.073472059, 0.9))
    csr.addobs((0.072958202, 0.9))
    csr.addobs((0.072457789, 1))
    csr.addobs((0.844657383, 0.9))
    csr.addobs((0.071496447, 0.9))
    csr.addobs((0.071034351, 1))
    csr.addobs((0.070584037, 0.9))
    csr.addobs((0.070145612, 0.6))
    csr.addobs((0.069717158, 1))
    csr.addobs((0.069298895, 0.5))
    csr.addobs((0.853830002, 0.3))
    csr.addobs((0.855015059, 1))
    csr.addobs((0.068101742, 1))
    csr.addobs((0.067721242, 0.9))
    csr.addobs((0.067348406, 0.1))
    csr.addobs((0.066983637, 0.3))
    csr.addobs((0.860565217, 0))
    csr.addobs((0.066277134, 0.6))
    csr.addobs((42.83659821, 0.4))
    csr.addobs((0.863622193, 0.3))
    csr.addobs((0.864597097, 1))
    csr.addobs((0.064949207, 0.1))
    csr.addobs((0.064633158, 0.6))
    csr.addobs((0.867408224, 0.9))
    csr.addobs((0.868308821, 0.9))
    csr.addobs((0.063721357, 0.9))
    csr.addobs((0.870070408, 0.4))
    csr.addobs((0.063140739, 1))
    csr.addobs((0.062858233, 0.6))
    csr.addobs((0.872580081, 1))
    csr.addobs((0.062307872, 0.9))
    csr.addobs((0.062040461, 1))
    csr.addobs((0.874968456, 0.6))
    csr.addobs((0.061517802, 0.9))
    csr.addobs((0.876488363, 1))
    csr.addobs((0.061012235, 0.6))
    csr.addobs((0.060765504, 0.9))
    csr.addobs((43.75649419, 0.4))
    csr.addobs((0.879389936, 1))
    csr.addobs((0.880083546, 1))
    csr.addobs((0.8807676, 1))
    csr.addobs((43.91880613, 0.9))

    alpha = 0.05 / 16.0
    lb, ub = csr.getci(alpha)

    # Compare bounds to confidence_sequence_robust_test.cc
    np.testing.assert_almost_equal(lb, 0.30209151281846858, 4)
    np.testing.assert_almost_equal(ub, 0.90219143188334106, 4)

    # Test brentq separately
    s = 139.8326745448
    thres = 6.46147
    memo = {0: 39.016179559463588, 1: 20.509121588503511, 2: 10.821991705197142}
    minmu = 0.0
    maxmu = 1.0

    res = so.root_scalar(
        f=lambda mu: csr.lower.logwealthmix(mu=mu, s=s, thres=thres, memo=memo) - thres,
        method="brentq",
        bracket=[minmu, maxmu],
    )

    # Compare to brentq in confidence_sequence_robust_test.cc
    np.testing.assert_almost_equal(res.root, 0.30209143008131789, 4)

    # Test that root of objective function is 0
    test_root = csr.lower.logwealthmix(mu=res.root, s=s, thres=thres, memo=memo) - thres
    np.testing.assert_almost_equal(test_root, 0.0, 2)

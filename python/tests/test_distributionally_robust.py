from distributionally_robust_data import OnlineCressieRead
from math import exp, pi
import numpy as np


def test_recompute_duals_lower():
    ocrl = OnlineCressieRead(alpha=0.05, tau=0.999)

    ws = np.random.RandomState(seed=42).exponential(size=10)
    rs = np.random.RandomState(seed=2112).random_sample(size=10)
    duals = []
    bounds = []

    ws_rs = [
        (0.4692680899768591, 0.08779271803562538),
        (3.010121430917521, 0.1488852932982503),
        (1.3167456935454493, 0.5579699034039329),
        (0.9129425537759532, 0.6202896863254631),
        (0.16962487046234628, 0.7284609393916186),
        (0.16959629191460518, 0.10028857734263497),
        (0.059838768608680676, 0.5165390922355259),
        (2.0112308644799395, 0.4596272470443479),
        (0.9190821536272645, 0.4352012556681023),
        (1.2312500617045903, 0.40365563207132593),
    ]

    np.testing.assert_almost_equal(list(zip(ws, rs)), ws_rs, 5)

    for (w, r) in zip(ws, rs):
        ocrl.update(1, w, r)
        ocrl.recomputeduals()
        if ocrl.duals[1][0] is None:
            duals.append((True, 0, 0, 0, 0))
        else:
            bounds.append(ocrl.duals[0][0])
            duals.append(
                (
                    False,
                    ocrl.duals[1][0]["kappastar"],
                    ocrl.duals[1][0]["gammastar"],
                    ocrl.duals[1][0]["betastar"],
                    ocrl.n,
                )
            )

    exp_bounds = [
        0.09662899139580064,
        0.04094541671274077,
        0.12209962713632538,
        0.1418901366388003,
        0.14012602622199424,
        0.10155119256007322,
        0.1846416198342555,
        0.20562281337616062,
        0.23301703596172654,
    ]

    np.testing.assert_almost_equal(exp_bounds, bounds, 5)

    exp_duals = [
        (True, 0, 0, 0, 0),
        (False, 0.186284935714629, -0.5242563567278763, 0, 1.999),
        (
            False,
            0.24176630719751424,
            -0.3939735949427358,
            -0.1283677781597634,
            2.997001,
        ),
        (
            False,
            0.2789701026811336,
            -0.5061803928309371,
            -0.11471449055314126,
            3.994003999,
        ),
        (
            False,
            0.28140131203664326,
            -0.43475483491188227,
            -0.16912405473103076,
            4.9900099950009995,
        ),
        (
            False,
            0.29153800750906095,
            -0.3748233156965521,
            -0.22291421513333443,
            5.985019985005999,
        ),
        (False, 0.37075115114133017, -0.7039218308392182, 0, 6.979034965020992),
        (False, 0.563270986745603, -0.9948022925290081, 0, 7.972055930055971),
        (False, 0.5602916549924593, -0.9880343238436395, 0, 8.964083874125915),
        (False, 0.5684515391242324, -1.0040272155608332, 0, 9.95511979025179),
    ]

    np.testing.assert_almost_equal(duals, exp_duals, 5)


def test_recompute_duals_upper():
    ocrl = OnlineCressieRead(alpha=0.05, tau=0.999)

    ws = np.random.RandomState(seed=42).exponential(size=10)
    rs = np.random.RandomState(seed=2112).random_sample(size=10)
    duals = []
    bounds = []

    for (w, r) in zip(ws, rs):
        ocrl.update(1, w, r)
        ocrl.recomputeduals(is_upper=True)
        if ocrl.duals[1][0] is None:
            duals.append((True, 0, 0, 0, 0))
        else:
            bounds.append(ocrl.duals[0][0])
            duals.append(
                (
                    False,
                    ocrl.duals[1][0]["kappastar"],
                    ocrl.duals[1][0]["gammastar"],
                    ocrl.duals[1][0]["betastar"],
                    ocrl.n,
                )
            )

    exp_bounds = [
        0.28128352388284217,
        0.5056614942571485,
        0.6220782066057806,
        0.75416164144364,
        0.8175722577850846,
        0.944281011930322,
        0.7625855039970183,
        0.7386659400005198,
        0.69289830401895,
    ]

    np.testing.assert_almost_equal(exp_bounds, bounds, 5)


def test_qlb():
    ocrl = OnlineCressieRead(alpha=0.05, tau=0.999)

    ws = np.random.RandomState(seed=42).exponential(size=10)
    rs = np.random.RandomState(seed=2112).random_sample(size=10)
    qlbs = []

    ws_rs = [
        (0.4692680899768591, 0.08779271803562538),
        (3.010121430917521, 0.1488852932982503),
        (1.3167456935454493, 0.5579699034039329),
        (0.9129425537759532, 0.6202896863254631),
        (0.16962487046234628, 0.7284609393916186),
        (0.16959629191460518, 0.10028857734263497),
        (0.059838768608680676, 0.5165390922355259),
        (2.0112308644799395, 0.4596272470443479),
        (0.9190821536272645, 0.4352012556681023),
        (1.2312500617045903, 0.40365563207132593),
    ]

    np.testing.assert_almost_equal(list(zip(ws, rs)), ws_rs, 5)

    for (w, r) in zip(ws, rs):
        ocrl.update(1, w, r)
        qlbs.append(ocrl.qlb(w, r))

    exp_qlbs = [
        1,
        0.13620517641052662,
        -0.17768396518176874,
        0.03202698335276157,
        0.20163624787093867,
        0.19427440609482105,
        0.22750472815940542,
        0.01392757858090217,
        0.10533233309112934,
        0.08141788541188416,
    ]

    np.testing.assert_almost_equal(qlbs, exp_qlbs, 5)

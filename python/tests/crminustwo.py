class CrMinusTwo:
    @staticmethod
    def estimate(datagen, wmin, wmax, rmin=0, rmax=1, raiseonerr=False, censored=False):
        from math import inf

        n, sumw, sumwsq, sumwr, sumwsqr, sumwany, sumwsqany = 0, 0, 0, 0, 0, 0, 0
        for c, w, r in datagen():
            n += c
            sumw += c * w
            sumwsq += c * w * w
            if r is not None:
                sumwany += c * w
                sumwsqany += c * w * w
                sumwr += c * w * r
                sumwsqr += c * w * w * r

        assert n > 0

        return CrMinusTwo.estimateimpl(
            n,
            sumw,
            sumwsq,
            sumwr,
            sumwsqr,
            sumwany,
            sumwsqany,
            wmin,
            wmax,
            rmin,
            rmax,
            raiseonerr,
            censored,
        )

    @staticmethod
    def estimateimpl(
        n,
        sumw,
        sumwsq,
        sumwr,
        sumwsqr,
        sumwany,
        sumwsqany,
        wmin,
        wmax,
        rmin=0,
        rmax=1,
        raiseonerr=False,
        censored=False,
    ):
        from math import inf

        assert wmin >= 0
        assert wmin < 1
        assert wmax > 1
        assert rmax >= rmin

        wfake = wmax if sumw < n else wmin

        if wfake == inf:
            gammastar = -(1 + n) / n
            betastar = 0
            gstar = 1 + 1 / n
        else:
            a = (wfake + sumw) / (1 + n)
            b = (wfake**2 + sumwsq) / (1 + n)
            assert a * a < b
            gammastar = (b - a) / (a * a - b)
            betastar = (1 - a) / (a * a - b)
            gstar = (n + 1) * (a - 1) ** 2 / (b - a * a)

        vhat = (-gammastar * sumwr - betastar * sumwsqr) / (1 + n)
        missing = max(0, 1 - (-gammastar * sumw - betastar * sumwsq) / (1 + n))

        if censored:
            # vhat = E[w r 1_{r is not None}] / E[w 1_{r is not None}]

            vnumhat = vhat
            vdenomhat = (-gammastar * sumwany - betastar * sumwsqany) / (1 + n)

            # Minimize[{ (x + a r) / (y + a),  y >= 0, 0 <= a <= m }, a]
            #
            # extrema is always at endpoints of interval

            vmincandidates = []
            vmaxcandidates = []
            if vdenomhat > 0:
                vmincandidates.append(vnumhat / vdenomhat)
                vmaxcandidates.append(vnumhat / vdenomhat)

            if vdenomhat + missing > 0:
                vmincandidates.append(
                    (vnumhat + missing * rmin) / (vdenomhat + missing)
                )
                vmaxcandidates.append(
                    (vnumhat + missing * rmax) / (vdenomhat + missing)
                )

            vmin = min(vmincandidates, default=None)
            vmax = max(vmaxcandidates, default=None)
            vhat = None if vmin is None or vmax is None else (vmin + vmax) / 2
        else:
            vmin = vhat + missing * rmin
            vmax = vhat + missing * rmax
            vhat += missing * (rmin + rmax) / 2

        vmin, vmax, vhat = (
            None if x is None else min(rmax, max(rmin, x)) for x in (vmin, vmax, vhat)
        )

        return vhat, {
            "primal": gstar,
            "gammastar": gammastar,
            "betastar": betastar,
            "vmin": vmin,
            "vmax": vmax,
            "num": n,
            "qfunc": lambda c, w, r: (c / (1 + n)) * (-gammastar - betastar * w),
        }

    @staticmethod
    def estimatediff(
        datagen,
        umin,
        umax,
        wmin,
        wmax,
        rmin=0,
        rmax=1,
        raiseonerr=False,
        censored=False,
    ):
        import numpy as np

        assert umin >= 0
        assert umin < 1
        assert umax > 1
        assert wmin >= 0
        assert wmin < 1
        assert wmax > 1
        assert rmax >= rmin
        assert not censored

        n, sumu, sumw, sumuw, sumusq, sumwsq = 0, 0, 0, 0, 0, 0
        sumuMwr, sumuuMwr, sumwuMwr = 0, 0, 0
        for c, u, w, r in datagen():
            n += c
            sumu += c * u
            sumw += c * w
            sumuw += c * u * w
            sumusq += c * u**2
            sumwsq += c * w**2
            sumuMwr += c * (u - w) * r
            sumuuMwr += c * u * (u - w) * r
            sumwuMwr += c * w * (u - w) * r

        assert n > 0

        ufake = umax if sumu < n else umin
        wfake = wmax if sumw < n else wmin

        ubar = (sumu + ufake) / (n + 1)
        usqbar = (sumusq + ufake**2) / (n + 1)
        uwbar = (sumuw + ufake * wfake) / (n + 1)
        wbar = (sumw + wfake) / (n + 1)
        wsqbar = (sumwsq + wfake**2) / (n + 1)

        A = np.array(
            [[-1, -ubar, -wbar], [-ubar, -usqbar, -uwbar], [-wbar, -uwbar, -wsqbar]],
            dtype="float64",
        )
        b = np.ones(3, dtype="float64")

        xstar = np.linalg.lstsq(A, b, rcond=-1)[0]
        beta, gamma, tau = xstar

        deltavhat = (-beta * sumuMwr - gamma * sumuuMwr - tau * sumwuMwr) / n
        missing = (
            -beta * (ufake - wfake)
            - gamma * (ufake**2 - ufake * wfake)
            - tau * (ufake * wfake - wfake**2)
        ) / (n + 1)

        deltavmin = deltavhat + min(rmin * missing, rmax * missing)
        deltavmax = deltavhat + max(rmin * missing, rmax * missing)
        deltavhat = (deltavmin + deltavmax) / 2

        deltavmin, deltavmax, deltavhat = (
            min(rmax - rmin, max(rmin - rmax, x))
            for x in (deltavmin, deltavmax, deltavhat)
        )

        qfunc = lambda c, u, w, r, n=n, b=beta, g=gamma, t=tau: (c / (n + 1)) * (
            -b - g * u - t * w
        )

        return deltavhat, {
            "deltavmin": deltavmin,
            "deltavmax": deltavmax,
            "num": n,
            "betastar": beta,
            "gammastar": gamma,
            "taustar": tau,
            "primal": -(n + 1) * (1 + beta + gamma + tau),
            "qfunc": qfunc,
        }

    @staticmethod
    def interval(datagen, wmin, wmax, alpha=0.05, rmin=0, rmax=1, raiseonerr=False):
        from math import inf, isclose, sqrt
        from scipy.stats import f

        assert wmin < 1
        assert wmax > 1
        assert rmin <= rmax

        n, sumw, sumwsq, sumwr, sumwsqr, sumwsqrsq = 0, 0, 0, 0, 0, 0

        for c, w, r in datagen():
            n += c
            sumw += c * w
            sumwsq += c * w**2
            sumwr += c * w * r
            sumwsqr += c * w**2 * r
            sumwsqrsq += c * w**2 * r**2
        assert n > 0

        return CrMinusTwo.intervalimpl(
            n,
            sumw,
            sumwsq,
            sumwr,
            sumwsqr,
            sumwsqrsq,
            wmin,
            wmax,
            alpha,
            rmin,
            rmax,
            raiseonerr,
        )

    @staticmethod
    def intervalimpl(
        n,
        sumw,
        sumwsq,
        sumwr,
        sumwsqr,
        sumwsqrsq,
        wmin,
        wmax,
        alpha=0.05,
        rmin=0,
        rmax=1,
        raiseonerr=False,
    ):
        from math import inf, isclose, sqrt
        from scipy.stats import f

        assert wmin < 1
        assert wmax > 1
        assert rmin <= rmax

        uncwfake = wmax if sumw < n else wmin
        if uncwfake == inf:
            uncgstar = 1 + 1 / n
        else:
            unca = (uncwfake + sumw) / (1 + n)
            uncb = (uncwfake**2 + sumwsq) / (1 + n)
            uncgstar = (n + 1) * (unca - 1) ** 2 / (uncb - unca * unca)
        Delta = f.isf(q=alpha, dfn=1, dfd=n)
        phi = (-uncgstar - Delta) / (2 * (n + 1))

        bounds = []
        for r, sign in ((rmin, 1), (rmax, -1)):
            candidates = []
            for wfake in (wmin, wmax):
                if wfake == inf:
                    x = sign * (r + (sumwr - sumw * r) / n)
                    y = (r * sumw - sumwr) ** 2 / (n * (1 + n)) - (
                        r**2 * sumwsq - 2 * r * sumwsqr + sumwsqrsq
                    ) / (1 + n)
                    z = phi + 1 / (2 * n)
                    if isclose(y * z, 0, abs_tol=1e-9):
                        y = 0

                    if z <= 0 and y * z >= 0:
                        kappa = sqrt(y / (2 * z))
                        if isclose(kappa, 0):
                            candidates.append((sign * r, None))
                        else:
                            gstar = x - sqrt(2 * y * z)
                            gamma = -kappa * (1 + n) / n + sign * (r * sumw - sumwr) / n
                            beta = -sign * r
                            candidates.append(
                                (
                                    gstar,
                                    {
                                        "kappastar": kappa,
                                        "betastar": beta,
                                        "gammastar": gamma,
                                        "wfake": wfake,
                                        # Q_{w,r} &= -\frac{\gamma + \beta w + w r}{(N+1) \kappa} \\
                                        "qfunc": lambda c, w, r, k=kappa, g=gamma, b=beta, s=sign, num=n: -c
                                        * (g + (b + s * r) * w)
                                        / ((num + 1) * k),
                                    },
                                )
                            )
                else:
                    barw = (wfake + sumw) / (1 + n)
                    barwsq = (wfake * wfake + sumwsq) / (1 + n)
                    barwr = sign * (wfake * r + sumwr) / (1 + n)
                    barwsqr = sign * (wfake * wfake * r + sumwsqr) / (1 + n)
                    barwsqrsq = (wfake * wfake * r * r + sumwsqrsq) / (1 + n)

                    if barwsq > barw**2:
                        x = barwr + (
                            (1 - barw) * (barwsqr - barw * barwr) / (barwsq - barw**2)
                        )
                        y = (barwsqr - barw * barwr) ** 2 / (barwsq - barw**2) - (
                            barwsqrsq - barwr**2
                        )
                        z = phi + (1 / 2) * (1 - barw) ** 2 / (barwsq - barw**2)
                        if isclose(y * z, 0, abs_tol=1e-9):
                            y = 0

                        if z <= 0 and y * z >= 0:
                            kappa = sqrt(y / (2 * z)) if y * z > 0 else 0
                            if isclose(kappa, 0):
                                candidates.append((sign * r, None))
                            else:
                                gstar = x - sqrt(2 * y * z)
                                beta = (
                                    -kappa * (1 - barw) - (barwsqr - barw * barwr)
                                ) / (barwsq - barw * barw)
                                gamma = -kappa - beta * barw - barwr
                                candidates.append(
                                    (
                                        gstar,
                                        {
                                            "kappastar": kappa,
                                            "betastar": beta,
                                            "gammastar": gamma,
                                            "wfake": wfake,
                                            # Q_{w,r} &= -\frac{\gamma + \beta w + w r}{(N+1) \kappa} \\
                                            "qfunc": lambda c, w, r, k=kappa, g=gamma, b=beta, s=sign, num=n: -c
                                            * (g + (b + s * r) * w)
                                            / ((num + 1) * k),
                                        },
                                    )
                                )

            best = min(candidates, key=lambda x: x[0])
            vbound = min(rmax, max(rmin, sign * best[0]))
            bounds.append((vbound, best[1]))

        return (bounds[0][0], bounds[1][0]), (bounds[0][1], bounds[1][1])

    @staticmethod
    def intervaldiff(
        datagen, umin, umax, wmin, wmax, alpha=0.05, rmin=0, rmax=1, raiseonerr=False
    ):
        import numpy as np
        from math import isclose, sqrt
        from scipy.stats import f

        assert umin >= 0
        assert umin < 1
        assert umax > 1
        assert wmin >= 0
        assert wmin < 1
        assert wmax > 1
        assert rmax >= rmin

        _, mle = CrMinusTwo.estimatediff(
            datagen, umin, umax, wmin, wmax, rmin, rmax, raiseonerr=raiseonerr
        )

        Delta = f.isf(q=alpha, dfn=1, dfd=mle["num"] - 1)
        phi = (-Delta - mle["primal"]) / (2 * (mle["num"] + 1))

        n, sumu, sumw, sumuw, sumusq, sumwsq = 0, 0, 0, 0, 0, 0
        sumuMwr, sumuuMwr, sumwuMwr, sumuMwsqrsq = 0, 0, 0, 0
        for c, u, w, r in datagen():
            n += c
            sumu += c * u
            sumw += c * w
            sumuw += c * u * w
            sumusq += c * u**2
            sumwsq += c * w**2
            sumuMwr += c * (u - w) * r
            sumuuMwr += c * u * (u - w) * r
            sumwuMwr += c * w * (u - w) * r
            sumuMwsqrsq += c * (u - w) ** 2 * r**2

        assert n > 0

        bounds = []
        for sign in (1, -1):
            candidates = []
            for ufake, wfake in ((u, w) for u in (umin, umax) for w in (wmin, wmax)):
                rex = rmin if sign * ufake >= sign * wfake else rmax

                baru = (sumu + ufake) / (n + 1)
                barw = (sumw + wfake) / (n + 1)
                barusq = (sumusq + ufake**2) / (n + 1)
                barwsq = (sumwsq + wfake**2) / (n + 1)
                baruw = (sumuw + ufake * wfake) / (n + 1)
                baruMwr = sign * (sumuMwr + (ufake - wfake) * rex) / (n + 1)
                baruuMwr = sign * (sumuuMwr + ufake * (ufake - wfake) * rex) / (n + 1)
                barwuMwr = sign * (sumwuMwr + wfake * (ufake - wfake) * rex) / (n + 1)
                baruMwsqrsq = (sumuMwsqrsq + (ufake - wfake) ** 2 * rex**2) / (n + 1)

                C = np.array(
                    [
                        [1.0, baru, barw],
                        [baru, barusq, baruw],
                        [barw, baruw, barwsq],
                    ],
                    dtype="float64",
                )
                d = np.array([baruMwr, baruuMwr, barwuMwr], dtype="float64")

                a = np.linalg.lstsq(C, np.ones(3), rcond=-1)[0]
                b = np.linalg.lstsq(C, d, rcond=-1)[0]
                x = np.sum(b)
                y = np.dot(d, b) - baruMwsqrsq
                z = phi - 0.5 + 0.5 * np.sum(a)

                if isclose(y * z, 0, abs_tol=1e-9):
                    y = 0

                if z <= 0 and y * z >= 0:
                    gstar = x - sqrt(2 * y * z)
                    kappa = sqrt(y / (2 * z)) if y * z > 0 else 0
                    beta, gamma, tau = -kappa * a - b

                    candidates.append(
                        (
                            gstar,
                            None
                            if isclose(kappa, 0)
                            else {
                                "kappastar": kappa,
                                "betastar": beta,
                                "gammastar": gamma,
                                "taustar": tau,
                                "ufake": ufake,
                                "wfake": wfake,
                                "rfake": rex,
                                "qfunc": lambda c, u, w, r, k=kappa, g=gamma, b=beta, t=tau, s=sign, num=n: -c
                                * (b + g * u + t * w + s * (u - w) * r)
                                / ((num + 1) * k),
                                "mle": mle,
                            },
                        )
                    )

            best = min(candidates, key=lambda x: x[0])
            vbound = min(rmax - rmin, max(rmin - rmax, sign * best[0]))
            bounds.append((vbound, best[1]))

        return (bounds[0][0], bounds[1][0]), (bounds[0][1], bounds[1][1]), candidates

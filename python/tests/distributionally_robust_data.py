from math import inf


class OnlineCressieRead:
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
        upper_bound=False,
    ):
        from math import inf, isclose, sqrt
        from scipy.stats import chi2

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
        Delta = chi2.isf(q=alpha, df=1)
        phi = (-uncgstar - Delta) / (2 * (n + 1))

        bounds = []

        r_sign = (rmax, -1) if upper_bound else (rmin, 1)
        for r, sign in (r_sign,):
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

        return (bounds[0][0],), (bounds[0][1],)

    def __init__(self, alpha, tau=1, wmin=0, wmax=inf):
        import numpy as np

        self.alpha = alpha
        self.tau = tau
        self.n = 0
        self.sumw = 0
        self.sumwsq = 0
        self.sumwr = 0
        self.sumwsqr = 0
        self.sumwsqrsq = 0
        self.wmin = wmin
        self.wmax = wmax

        self.duals = None
        self.mleduals = None

    def update(self, c, w, r):
        if c > 0:
            assert (
                w + 1e-6 >= self.wmin and w <= self.wmax + 1e-6
            ), "w = {} < {} < {}".format(self.wmin, w, self.wmax)
            assert r >= 0 and r <= 1, "r = {}".format(r)

            decay = self.tau**c
            self.n = decay * self.n + c
            self.sumw = decay * self.sumw + c * w
            self.sumwsq = decay * self.sumwsq + c * w**2
            self.sumwr = decay * self.sumwr + c * w * r
            self.sumwsqr = decay * self.sumwsqr + c * (w**2) * r
            self.sumwsqrsq = decay * self.sumwsqrsq + c * (w**2) * (r**2)

            self.duals = None
            self.mleduals = None

        return self

    def recomputeduals(self, is_upper=False):
        from crminustwo import CrMinusTwo as CrMinusTwo

        self.duals = self.intervalimpl(
            self.n,
            self.sumw,
            self.sumwsq,
            self.sumwr,
            self.sumwsqr,
            self.sumwsqrsq,
            self.wmin,
            self.wmax,
            self.alpha,
            raiseonerr=True,
            upper_bound=is_upper,
        )

    def qlb(self, w, r):
        if self.duals is None:
            self.recomputeduals()

            assert self.duals is not None

        return self.duals[1][0]["qfunc"](1, w, r) if self.duals[1][0] is not None else 1

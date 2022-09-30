from cmath import sqrt


class incremental_f_sum:
    """Incremental version of https://en.wikipedia.org/wiki/Kahan_summation_algorithm"""

    def __init__(self):
        self.partials = []

    def __iadd__(self, x):
        i = 0
        for y in self.partials:
            if abs(x) < abs(y):
                x, y = y, x
            hi = x + y
            lo = y - (hi - x)
            if lo:
                self.partials[i] = lo
                i += 1
            x = hi
        self.partials[i:] = [x]
        return self

    def __add__(self, other):
        result = incremental_f_sum()
        result.partials = deepcopy(self.partials)
        for y in other.partials:
            result += y
        return result

    def __float__(self):
        return sum(self.partials, 0.0)


class confidence_sequence(object):
    def __init__(self, rmin=0, rmax=1, adjust=True, eta=1.1, s=1.1):
        super().__init__()

        assert rmin <= rmax, (rmin, rmax)

        self.rho = 1
        self.eta = eta
        self.s = s
        self.rmin = rmin
        self.rmax = rmax
        self.adjust = adjust

        self.t = 0

        self.sumwsqrsq = incremental_f_sum()
        self.sumwsqr = incremental_f_sum()
        self.sumwsq = incremental_f_sum()
        self.sumwr = incremental_f_sum()
        self.sumw = incremental_f_sum()
        self.sumwrxhatlow = incremental_f_sum()
        self.sumwxhatlow = incremental_f_sum()
        self.sumxhatlowsq = incremental_f_sum()
        self.sumwrxhathigh = incremental_f_sum()
        self.sumwxhathigh = incremental_f_sum()
        self.sumxhathighsq = incremental_f_sum()

    def addobs(self, w, r, p_drop=0, n_drop=None):
        assert w >= 0
        assert 0 <= p_drop < 1
        assert n_drop is None or n_drop >= 0

        if not self.adjust:
            r = min(self.rmax, max(self.rmin, r))
        else:
            self.rmin = min(self.rmin, r)
            self.rmax = max(self.rmax, r)

        if n_drop is None:
            n_drop = p_drop / (1 - p_drop)

        if n_drop > 0:
            import scipy.special as sc

            # we have to simulate presenting n_drop events with w=0 in a row, which we can do in closed form
            # Sum[(a/(b + s))^2, { s, 0, n - 1 }]
            # a^2 PolyGamma[1,b]-a^2 PolyGamma[1,b+n]

            sumXlow = (float(self.sumwr) - float(self.sumw) * self.rmin) / (
                self.rmax - self.rmin
            )
            alow = sumXlow + 1 / 2
            blow = self.t + 1
            self.sumxhatlowsq += alow**2 * (
                sc.polygamma(1, blow).item() - sc.polygamma(1, blow + n_drop).item()
            )

            sumXhigh = (float(self.sumw) * self.rmax - float(self.sumwr)) / (
                self.rmax - self.rmin
            )
            ahigh = sumXhigh + 1 / 2
            bhigh = self.t + 1
            self.sumxhathighsq += ahigh**2 * (
                sc.polygamma(1, bhigh).item() - sc.polygamma(1, bhigh + n_drop).item()
            )

            self.t += n_drop

        sumXlow = (float(self.sumwr) - float(self.sumw) * self.rmin) / (
            self.rmax - self.rmin
        )
        Xhatlow = (sumXlow + 1 / 2) / (self.t + 1)
        sumXhigh = (float(self.sumw) * self.rmax - float(self.sumwr)) / (
            self.rmax - self.rmin
        )
        Xhathigh = (sumXhigh + 1 / 2) / (self.t + 1)

        w /= 1 - p_drop

        self.sumwsqrsq += (w * r) ** 2
        self.sumwsqr += w**2 * r
        self.sumwsq += w**2
        self.sumwr += w * r
        self.sumw += w
        self.sumwrxhatlow += w * r * Xhatlow
        self.sumwxhatlow += w * Xhatlow
        self.sumxhatlowsq += Xhatlow**2
        self.sumwrxhathigh += w * r * Xhathigh
        self.sumwxhathigh += w * Xhathigh
        self.sumxhathighsq += Xhathigh**2

        self.t += 1
        # print('r: ' + str(r) + ', w: ' + str(w))

    def getci(self, alpha):
        if self.t == 0 or self.rmin == self.rmax:
            return [self.rmin, self.rmax]

        sumvlow = (
            (
                float(self.sumwsqrsq)
                - 2 * self.rmin * float(self.sumwsqr)
                + self.rmin**2 * float(self.sumwsq)
            )
            / (self.rmax - self.rmin) ** 2
            - 2
            * (float(self.sumwrxhatlow) - self.rmin * float(self.sumwxhatlow))
            / (self.rmax - self.rmin)
            + float(self.sumxhatlowsq)
        )
        sumXlow = (float(self.sumwr) - float(self.sumw) * self.rmin) / (
            self.rmax - self.rmin
        )
        l = self.lblogwealth(
            t=self.t, sumXt=sumXlow, v=sumvlow, eta=self.eta, s=self.s, alpha=alpha / 2
        )

        sumvhigh = (
            (
                float(self.sumwsqrsq)
                - 2 * self.rmax * float(self.sumwsqr)
                + self.rmax**2 * float(self.sumwsq)
            )
            / (self.rmax - self.rmin) ** 2
            + 2
            * (float(self.sumwrxhathigh) - self.rmax * float(self.sumwxhathigh))
            / (self.rmax - self.rmin)
            + float(self.sumxhathighsq)
        )
        sumXhigh = (float(self.sumw) * self.rmax - float(self.sumwr)) / (
            self.rmax - self.rmin
        )
        u = 1 - self.lblogwealth(
            t=self.t,
            sumXt=sumXhigh,
            v=sumvhigh,
            eta=self.eta,
            s=self.s,
            alpha=alpha / 2,
        )
        return self.rmin + l * (self.rmax - self.rmin), self.rmin + u * (
            self.rmax - self.rmin
        )

    def lblogwealth(self, *, t, sumXt, v, eta, s, alpha):
        import scipy.special as sc
        from math import log, sqrt

        zeta_s = sc.zeta(s)
        v = max(v, 1)
        gamma1 = (eta ** (1 / 4) + eta ** (-1 / 4)) / sqrt(2)
        gamma2 = (sqrt(eta) + 1) / 2
        assert log(eta * v, eta) + 1 > 0, 1 + log(eta * v, eta)
        ll = s * log(log(eta * v, eta) + 1) + log(zeta_s / alpha)

        return max(
            0,
            (sumXt - sqrt(gamma1**2 * ll * v + gamma2**2 * ll**2) - gamma2 * ll)
            / t,
        )

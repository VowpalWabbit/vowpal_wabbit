class LowerCSBase(object):
    def __init__(self):
        super().__init__()

    def addobs(self, x):
        raise NotImplementedError

    def getci(self, alpha):
        raise NotImplementedError


class CountableDiscreteBase(LowerCSBase):
    def __init__(self, *, lambdamax=1 / 2, xi=8 / 5, **kwargs):
        from math import log1p, exp
        import scipy.special as sc

        super().__init__(**kwargs)

        assert 0 < lambdamax <= 1 + sc.lambertw(-exp(-2))
        assert 1 < xi

        self.lambdamax = lambdamax
        self.xi = xi
        self.logxi = log1p(xi - 1)

    def getci(self, alpha):
        return self.__lblogwealth(alpha=alpha)

    def __get_lam_sqrttp1(self, j):
        from math import log, exp

        logden = (j + 1 / 2) * self.logxi - 1 / 2 * log(self.t + 1)
        return self.lambdamax * exp(-logden)

    def get_log_weight(self, j):
        raise NotImplementedError

    def get_log_remaining_weight(self, j):
        raise NotImplementedError

    def __getVimpl(self, memo, j):
        if j not in memo:
            memo[j] = self.getV(self.__get_lam_sqrttp1(j))
        return memo[j]

    def logwealthmix(self, *, mu, s, thres, memo):
        from math import sqrt
        import numpy as np
        from scipy.special import logsumexp

        sqrttp1 = sqrt(self.t + 1)
        y = s / sqrttp1 - (self.t / sqrttp1) * mu

        logEs = np.array(
            [self.__get_lam_sqrttp1(j) * y - v for j, v in sorted(memo.items())]
        )
        logws = np.array([self.get_log_weight(j) for j, _ in sorted(memo.items())])
        j = len(logEs)

        while True:
            lowerbound = logsumexp(a=logEs + logws)

            if lowerbound >= thres:
                return lowerbound

            # quasiconcave after the maximum, otherwise lower bound variance with 0
            logupperexp = (
                logEs[-1]
                if logEs[-1] < max(logEs[:-1])
                else self.__get_lam_sqrttp1(j) * y
            )
            upperbound = logsumexp(
                a=[lowerbound, self.get_log_remaining_weight(j) + logupperexp]
            )

            if upperbound < thres:
                return upperbound

            v = self.__getVimpl(memo, j)
            logEs = np.append(logEs, self.__get_lam_sqrttp1(j) * y - v)
            logws = np.append(logws, self.get_log_weight(j))
            j += 1

    def __lblogwealth(self, *, alpha):
        from math import log
        import scipy.optimize as so

        assert 0 < alpha < 1, alpha
        thres = -log(alpha)

        minmu = 0
        s = self.getS()
        memo = {j: self.getV(self.__get_lam_sqrttp1(j)) for j in range(2)}

        logwealthminmu = self.logwealthmix(mu=minmu, s=s, thres=thres, memo=memo)
        if logwealthminmu <= thres:
            return minmu

        maxmu = 1
        logwealthmaxmu = self.logwealthmix(mu=maxmu, s=s, thres=thres, memo=memo)
        if logwealthmaxmu >= thres:
            return maxmu

        res = so.root_scalar(
            f=lambda mu: self.logwealthmix(mu=mu, s=s, thres=thres, memo=memo) - thres,
            method="brentq",
            bracket=[minmu, maxmu],
        )
        assert res.converged, res
        return res.root

    def getS(self):
        raise NotImplementedError

    def getV(self, lamsqrttp1):
        raise NotImplementedError


class GTilde(object):
    def __init__(self, *, k, **kwargs):
        from math import log

        assert 1 < k < 2

        self.k = k

        self.logk = log(k)
        self.sumX = 0
        self.sumlowv = 0
        self.summidv = 0
        self.sumvhisto = {}
        self.t = 0

        super().__init__(**kwargs)

    def __histovariance(self, *, hist, lamsqrttp1):
        from math import log1p, sqrt

        sqrttp1 = sqrt(self.t + 1)
        return sum(
            c
            * (
                (lamsqrttp1 * (self.k - 1) * xds / (1 + lamsqrttp1 * self.k * xds)) ** 2
                if strongterm
                else lamsqrttp1 * xds - log1p(lamsqrttp1 * xds)
            )
            for ((n, strongterm), c) in hist.items()
            for xraw in (self.k**n,)
            for xds in (xraw / sqrttp1,)
        )

    def __histoinsert(self, *, hist, x):
        from math import log, floor

        n = int(floor(log(x) / self.logk))
        x1 = self.k**n
        alpha = (self.k * x1 - x) / ((self.k - 1) * x1)
        # TODO: Kahan summation
        hist[(n, False)] = alpha + hist.get((n, False), 0)
        hist[(n + 1, False)] = 1 - alpha + hist.get((n + 1, False), 0)
        hist[(n, True)] = -(1 / 2) * alpha * (1 - alpha) + hist.get((n, True), 0)

    def addobs(self, x):
        assert x >= 0

        Xhat = (self.sumX + 1 / 2) / (self.t + 1)

        error = x - min(1, Xhat)
        if error <= 0:
            self.sumlowv += error**2
        elif error <= 1:
            self.summidv += error**2
        else:
            self.__histoinsert(hist=self.sumvhisto, x=error)

        self.sumX += x
        self.t += 1

    def getS(self):
        return self.sumX

    def getV(self, lamsqrttp1):
        from math import sqrt

        vlow = self.sumlowv / ((self.t + 1) - sqrt(self.t + 1) * lamsqrttp1)
        vmid = self.summidv / (self.t + 1)

        return (1 / 2) * (lamsqrttp1**2) * (vlow + vmid) + self.__histovariance(
            hist=self.sumvhisto, lamsqrttp1=lamsqrttp1
        )


class RobustMixture(CountableDiscreteBase):
    def __init__(self, *, eta=0.95, r=2, k=3 / 2, **kwargs):
        from math import log1p
        import scipy.special as sc

        assert 0 < eta < 1
        assert r > 1

        self.eta = eta
        self.r = r
        self.gtilde = GTilde(k=k)
        polylog_val = 1.4406337969700393  # mpmath.fp.polylog(r, eta)
        self.scalefac = 1 / 2 * (1 + polylog_val / (eta * sc.zeta(r)))
        assert 0 < self.scalefac < 1, self.scalefac
        self.logscalefac = log1p(self.scalefac - 1)
        self.t = 0

        super().__init__(**kwargs)

        self.logxim1 = log1p(self.xi - 2)

    def get_log_weight(self, j):
        return self.logscalefac + self.logxim1 - (1 + j) * self.logxi

    def get_log_remaining_weight(self, j):
        return self.logscalefac - j * self.logxi

    def getS(self):
        return self.gtilde.getS()

    def getV(self, lamsqrttp1):
        return self.gtilde.getV(lamsqrttp1)

    def addobs(self, x):
        self.t += 1
        return self.gtilde.addobs(x)


class OffPolicyCS(object):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)

    def addobs(self, x):
        w, r = x
        assert w >= 0
        assert 0 <= r <= 1

        self.lower.addobs(w * r)
        self.upper.addobs(w * (1 - r))

    def getci(self, alpha):
        l, u = self.lower.getci(alpha / 2), self.upper.getci(alpha / 2)

        return l, 1 - u


class DDRM(OffPolicyCS):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self.lower = RobustMixture(**kwargs)
        self.upper = RobustMixture(**kwargs)

#include "vw/core/confidence_sequence.h"
#include "vw/core/model_utils.h"

#include <cassert>
#include <cmath>

namespace VW
{
namespace confidence_sequence
{
    double incrementalfsum()
    {
        return 0.0;
    }

    double IntervalImpl::loggamma(double a)
    {
        return 0;
    }

    double IntervalImpl::gammainc(double a, double x)
    {
        return 0;
    }

    double IntervalImpl::loggammalowerinc(double a, double x)
    {
        return std::log(gammainc(a, x) + loggamma(a));
    }

    double IntervalImpl::logwealth(double s, double v, double rho)
    {
        assert(s + v + rho > 0);
        assert(rho > 0);
        return s + v + rho * std::log(rho) - (v + rho) * std::log(s + v + rho) + loggammalowerinc(v + rho, s + v + rho) - loggammalowerinc(rho, rho);
    }

    std::pair<bool, double> IntervalImpl::root_scalar(double sumXt, double v, double rho, double thres, double minmu, double maxmu)
    {
        return std::make_pair(false, 0.0);
    }

    double IntervalImpl::lblogwealth(double sumXt, double v, double rho, double alpha)
    {
        assert(0 < alpha && alpha < 1);
        double thres = -std::log(alpha);

        double minmu = 0.0;
        double logwealthminmu = logwealth(sumXt, v, rho);

        if (logwealthminmu <= thres)
        {
            return minmu;
        }

        double maxmu = std::min(1.0, sumXt / t);
        double logwealthmaxmu = logwealth(sumXt - t * maxmu, v, rho);

        if (logwealthmaxmu >= thres)
        {
            return maxmu;
        }

        std::pair<bool, double> res = root_scalar(sumXt, v, rho, thres, minmu, maxmu); 

        assert (res.first);
        return res.second;
    }

    IntervalImpl::IntervalImpl(double rmin = 0, double rmax = 1, bool adjust = true)
        : rmin(rmin), rmax(rmax), adjust(adjust)
    {
    }

    void IntervalImpl::add(double w, double r)
    {
        assert(w >= 0);

        if (!adjust)
        {
            r = std::min(rmax, std::max(rmin, r));
        }
        else
        {
            rmin = std::min(rmin, r);
            rmax = std::max(rmax, r);
        }
        
        double sumXlow = (sumwr - sumw * rmin) / (rmax - rmin);
        double Xhatlow = (sumXlow + 0.5) / (t + 1);
        double sumXhigh = (sumw * rmax - sumwr) / (rmax - rmin);
        double Xhathigh = (sumXhigh + 0.5) / (t + 1);

        sumwsqrsq += std::pow(w * r, 2);
        sumwsqr += std::pow(w, 2) * r;
        sumwsq += std::pow(w, 2);
        sumwr += w * r;
        sumw += w;
        sumwrxhatlow += w * r * Xhatlow;
        sumwxhatlow += w * Xhatlow;
        sumxhatlowsq += std::pow(Xhatlow, 2);
        sumwrxhathigh += w * r * Xhathigh;
        sumwxhathigh += w * Xhathigh;
        sumxhathighsq += std::pow(Xhathigh, 2);

        t++;
    }

    std::pair<double, double> IntervalImpl::get(double alpha)
    {
        if (t == 0 || rmin == rmax)
        {
            return std::make_pair(0, 0); // Should return None?
        }

        double sumvlow = (sumwsqrsq - 2 * rmin * sumwsqr + std::pow(rmin, 2) * sumwsq) / std::pow(rmax - rmin, 2) - 2 * (sumwrxhatlow - rmin * sumwxhatlow) / (rmax - rmin) + sumxhatlowsq;
        double sumXlow = (sumwr - sumw * rmin) / (rmax - rmin);
        double l = lblogwealth(sumXlow, sumvlow, rho, alpha / 2);

        double sumvhigh = (sumwsqrsq - 2 * rmax * sumwsqr + std::pow(rmax, 2) * sumwsq) / std::pow(rmax - rmin, 2) + 2 * (sumwrxhathigh - rmax * sumwxhathigh) / (rmax - rmin) + sumxhathighsq;
        double sumXhigh = (sumw * rmax - sumwr) / (rmax - rmin);
        double u = 1 - lblogwealth(sumXhigh, sumvhigh, rho, alpha / 2);

        return std::make_pair(rmin + l * (rmax - rmin), rmin + u * (rmax - rmin));
    }
}  // namespace confidence_sequence

namespace model_utils
{
size_t read_model_field(io_buf& io, VW::confidence_sequence::IntervalImpl& im)
{
  size_t bytes = 0;
  bytes += read_model_field(io, im.rho);
  bytes += read_model_field(io, im.rmin);
  bytes += read_model_field(io, im.rmax);
  bytes += read_model_field(io, im.adjust);
  bytes += read_model_field(io, im.t);
  bytes += read_model_field(io, im.sumwsqrsq);
  bytes += read_model_field(io, im.sumwsqr);
  bytes += read_model_field(io, im.sumwsq);
  bytes += read_model_field(io, im.sumwr);
  bytes += read_model_field(io, im.sumw);
  bytes += read_model_field(io, im.sumwrxhatlow);
  bytes += read_model_field(io, im.sumwxhatlow);
  bytes += read_model_field(io, im.sumxhatlowsq);
  bytes += read_model_field(io, im.sumwrxhathigh);
  bytes += read_model_field(io, im.sumwxhathigh);
  bytes += read_model_field(io, im.sumxhathighsq);
  return bytes;
}

size_t write_model_field(io_buf& io, const VW::confidence_sequence::IntervalImpl& im, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, im.rho, upstream_name + "_rho", text);
  bytes += write_model_field(io, im.rmin, upstream_name + "_rmin", text);
  bytes += write_model_field(io, im.rmax, upstream_name + "_rmax", text);
  bytes += write_model_field(io, im.adjust, upstream_name + "_adjust", text);
  bytes += write_model_field(io, im.t, upstream_name + "_t", text);
  bytes += write_model_field(io, im.sumwsqrsq, upstream_name + "_sumwsqrsq", text);
  bytes += write_model_field(io, im.sumwsqr, upstream_name + "_sumwsqr", text);
  bytes += write_model_field(io, im.sumwsq, upstream_name + "_sumwsq", text);
  bytes += write_model_field(io, im.sumwr, upstream_name + "_sumwr", text);
  bytes += write_model_field(io, im.sumw, upstream_name + "_sumw", text);
  bytes += write_model_field(io, im.sumwrxhatlow, upstream_name + "_sumwrxhatlow", text);
  bytes += write_model_field(io, im.sumwxhatlow, upstream_name + "_sumwxhatlow", text);
  bytes += write_model_field(io, im.sumxhatlowsq, upstream_name + "_sumxhatlowsq", text);
  bytes += write_model_field(io, im.sumwrxhathigh, upstream_name + "_sumwrxhathigh", text);
  bytes += write_model_field(io, im.sumwxhathigh, upstream_name + "_sumwxhathigh", text);
  bytes += write_model_field(io, im.sumxhathighsq, upstream_name + "_sumxhathighsq", text);
  return bytes;
}
}  // namespace model_utils
}  // namespace VW
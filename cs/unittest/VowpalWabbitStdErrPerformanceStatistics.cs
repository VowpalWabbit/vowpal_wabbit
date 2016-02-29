using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace cs_unittest
{
    class VowpalWabbitStdErrPerformanceStatistics
    {
        public double? AverageLoss { get; set; }

        public double? BestConstant { get; set; }

        public double? BestConstantLoss { get; set; }

        public ulong? NumberOfExamplesPerPass { get; set; }

        public ulong? TotalNumberOfFeatures { get; set; }

        public double? WeightedExampleSum { get; set; }

        public double? WeightedLabelSum { get; set; }
    }
}

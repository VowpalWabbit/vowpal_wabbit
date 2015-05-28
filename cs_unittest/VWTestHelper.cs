using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Research.MachineLearning;
using System.Globalization;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace cs_unittest
{
    internal static class VWTestHelper
    {
        internal static void AssertEqual(VowpalWabbitPerformanceStatistics expected, VowpalWabbitPerformanceStatistics actual)
        {
            Assert.AreEqual(expected.NumberOfExamplesPerPass, actual.NumberOfExamplesPerPass);
            Assert.AreEqual(expected.TotalNumberOfFeatures, actual.TotalNumberOfFeatures);
            Assert.AreEqual(expected.AverageLoss, actual.AverageLoss, 1e-5);
            Assert.AreEqual(expected.BestConstant, actual.BestConstant, 1e-5);
            Assert.AreEqual(expected.BestConstantLoss, actual.BestConstantLoss, 1e-5);
            Assert.AreEqual(expected.WeightedExampleSum, actual.WeightedExampleSum, 1e-5);
            Assert.AreEqual(expected.WeightedLabelSum, actual.WeightedLabelSum, 1e-5);
        }

        internal static VowpalWabbitPerformanceStatistics ReadPerformanceStatistics(string filename)
        {
            var lines = File.ReadAllLines(filename);
            var stats = new VowpalWabbitPerformanceStatistics()
            {
                NumberOfExamplesPerPass = FindULongEntry(lines, "number of examples per pass = "),
                TotalNumberOfFeatures = FindULongEntry(lines, "total feature number = "),
                AverageLoss = FindDoubleEntry(lines, "average loss = "),
                BestConstant = FindDoubleEntry(lines, "best constant = "),
                BestConstantLoss = FindDoubleEntry(lines, "best constant's loss = "),
                WeightedExampleSum = FindDoubleEntry(lines, "weighted example sum = "),
                WeightedLabelSum = FindDoubleEntry(lines, "weighted label sum = ")
            };

            return stats;
        }

        private static double FindDoubleEntry(string[] lines, string label)
        {
            var candidate = lines.FirstOrDefault(l => l.StartsWith(label));

            if (candidate == null)
            {
                return 0.0;
            }

            var ret = 0.0;
            if (double.TryParse(candidate.Substring(label.Length), NumberStyles.Float, CultureInfo.InvariantCulture, out ret))
            {
                return ret;   
            }

            return 0.0;
        }

        private static ulong FindULongEntry(string[] lines, string label)
        {
            var candidate = lines.FirstOrDefault(l => l.StartsWith(label));

            if (candidate == null)
            {
                return 0L;
            }

            ulong ret = 0L;
            if (ulong.TryParse(candidate.Substring(label.Length), NumberStyles.Float, CultureInfo.InvariantCulture, out ret))
            {
                return ret;
            }

            return 0L;
        }
    }
}

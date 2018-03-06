using System;
using System.Globalization;
using System.IO;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using VW;
using VW.Serializer;

namespace cs_test
{
    internal static class VWTestHelper
    {
        internal static void FuzzyEqual(double? expected, double actual, double epsilon, string message)
        {
            if (expected == null)
                return;

            // from test/RunTests
            var delta = Math.Abs(expected.Value - actual);

            if (delta > epsilon) {
                // We have a 'big enough' difference, but this difference
                // may still not be meaningful in all contexts:

                // Big numbers should be compared by ratio rather than
                // by difference

                // Must ensure we can divide (avoid div-by-0)
                if (Math.Abs(actual) <= 1.0) {
                    // If numbers are so small (close to zero),
                    // ($delta > $Epsilon) suffices for deciding that
                    // the numbers are meaningfully different
                    Assert.Fail(string.Format("{0} vs {1}: delta={2} > Epsilon={3}: {4}",
                        expected, actual, delta, epsilon, message));
                }

                // Now we can safely divide (since abs($word2) > 0)
                // and determine the ratio difference from 1.0
                var ratio_delta = Math.Abs(expected.Value / actual - 1.0);
                if (ratio_delta > epsilon) {
                    Assert.Fail(string.Format("{0} vs {1}: delta={2} > Epsilon={3}: {4}",
                        expected, actual, delta, epsilon, message));
                }
            }
        }

        internal static void AssertEqual(VowpalWabbitPerformanceStatistics expected, VowpalWabbitPerformanceStatistics actual)
        {
            if (expected.TotalNumberOfFeatures != actual.TotalNumberOfFeatures)
            {
                Console.Error.WriteLine(
                    "Warning: total number of features differs. Expected: {0} vs. actual: {1}",
                    expected.TotalNumberOfFeatures,
                    actual.TotalNumberOfFeatures);
            }

            Assert.AreEqual(expected.NumberOfExamplesPerPass, actual.NumberOfExamplesPerPass, "NumberOfExamplesPerPass");

            FuzzyEqual(expected.AverageLoss, actual.AverageLoss, 1e-3, "AverageLoss");
            FuzzyEqual(expected.BestConstant, actual.BestConstant, 1e-3, "BestConstant");
            // TODO: something weir'd is happening here. BestConstantsLoss is 0 if using RunAll
            // has the proper value if just the unit test is run
            //Console.WriteLine(expected.BestConstantLoss + " vs. " + actual.BestConstantLoss);
            //Assert.AreEqual(expected.BestConstantLoss, actual.BestConstantLoss, 1e-5);
            FuzzyEqual(expected.WeightedExampleSum, actual.WeightedExampleSum, 1e-3, "WeightedExampleSum");
            FuzzyEqual(expected.WeightedLabelSum, actual.WeightedLabelSum, 1e-3, "WeightedLabelSum");
        }
    }
}

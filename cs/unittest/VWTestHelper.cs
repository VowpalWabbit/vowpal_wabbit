using System;
using System.Globalization;
using System.IO;
using System.Linq;
using Antlr4.Runtime;
using Antlr4.Runtime.Atn;
using Antlr4.Runtime.Tree;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using VW;
using VW.Serializer;

namespace cs_unittest
{
    internal static class VWTestHelper
    {
        internal static void ParseInput(string text, IParseTreeListener listener)
        {
            ParseInput(new AntlrInputStream(text), listener);
        }

        internal static void ParseInput(Stream stream, IParseTreeListener listener)
        {
            ParseInput(new UnbufferedCharStream(stream), listener);
        }

        internal static void ParseInput(ICharStream stream, IParseTreeListener listener)
        {
            // optimized for memory consumption
            var lexer = new VowpalWabbitLexer(stream)
            {
                TokenFactory = new CommonTokenFactory(copyText: true)
            };

            var tokens = new UnbufferedTokenStream(lexer);
            var parser = new VowpalWabbitParser(tokens)
            {
                // Note; don't disable, as it is required to access the line
                // BuildParseTree = false,
            };
            // fast than LL(*)
            parser.Interpreter.PredictionMode = PredictionMode.Sll;

            parser.AddParseListener(listener);
            parser.AddErrorListener(new TestErrorListener());
            parser.start();
        }

        internal static void Learn<T, TListener>(string args, string inputFile, string stderrFile)
            where TListener : VowpalWabbitListenerToEvents<T>, new()
        {
            using (var vw = new VowpalWabbit<T>(args))
            using (var validate = new VowpalWabbitExampleValidator<T>(args))
            {
                var listener = new TListener();
                listener.Created = (line, data, label) =>
                {
                    if (data == null)
                    {
                        Assert.Fail("got empty example");
                    }

                    validate.Validate(line, data, label);
                    vw.Learn(data, label);
                };
                VWTestHelper.ParseInput(File.OpenRead(inputFile), listener);

                AssertEqual(stderrFile, vw.Native.PerformanceStatistics);
            }
        }

        internal static void Predict<TData, TListener>(string args, string inputFile, string referenceFile = null)
            where TData : BaseData
            where TListener : VowpalWabbitListenerToEvents<TData>, new()
        {
            float[] references = null;
            var index = 0;

            if (referenceFile != null)
            {
                references = File.ReadAllLines(referenceFile)
                    .Select(l => float.Parse(l.Split(' ')[0], CultureInfo.InvariantCulture))
                    .ToArray();
            }

            using (var vwRef = new VowpalWabbit(args))
            using (var vwModel = new VowpalWabbitModel(args))
            using (var vwValidate = new VowpalWabbit(args))
            using (var vwInMemoryShared2 = new VowpalWabbit<TData>(new VowpalWabbitSettings { Model = vwModel }))
            using (var validate = new VowpalWabbitExampleValidator<TData>(args))
            {
                var listener = new TListener();
                listener.Created = (line, x, label) =>
                {
                    validate.Validate(line, x, label);

                    var expectedDynamic = vwRef.Predict(x.Line, VowpalWabbitPredictionType.Dynamic);
                    Assert.IsInstanceOfType(expectedDynamic, typeof(float));
                    var expected = vwRef.Predict(x.Line, VowpalWabbitPredictionType.Scalar);

                    var actual = vwInMemoryShared2.Predict(x, VowpalWabbitPredictionType.Scalar, label);

                    Assert.AreEqual((float)expectedDynamic, actual, 1e-5);
                    Assert.AreEqual(expected, actual, 1e-5);

                    if (references != null)
                        Assert.AreEqual(references[index++], actual, 1e-5);
                };
            }
        }

        internal static void AssertEqual(string expectedFile, VowpalWabbitPerformanceStatistics actual)
        {
            var expectedPerformanceStatistics = ReadPerformanceStatistics(expectedFile);
            AssertEqual(expectedPerformanceStatistics, actual);
        }

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

        internal static void AssertEqual(VowpalWabbitStdErrPerformanceStatistics expected, VowpalWabbitPerformanceStatistics actual)
        {
            if (expected.TotalNumberOfFeatures != actual.TotalNumberOfFeatures)
            {
                Console.Error.WriteLine(
                    "Warning: total number of features differs. Expected: {0} vs. actual: {1}",
                    expected.TotalNumberOfFeatures,
                    actual.TotalNumberOfFeatures);
            }

            if (expected.NumberOfExamplesPerPass != null)
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

        internal static VowpalWabbitStdErrPerformanceStatistics ReadPerformanceStatistics(string filename)
        {
            var lines = File.ReadAllLines(filename);

            var numExamples = FindULongEntry(lines, "number of examples per pass = ");

            if (numExamples == 0)
                numExamples = FindULongEntry(lines, "number of examples = ");

            var stats = new VowpalWabbitStdErrPerformanceStatistics()
            {
                NumberOfExamplesPerPass = numExamples,
                TotalNumberOfFeatures = FindULongEntry(lines, "total feature number = "),
                AverageLoss = FindAverageLossEntry(lines),
                BestConstant = FindDoubleEntry(lines, "best constant = "),
                BestConstantLoss = FindDoubleEntry(lines, "best constant's loss = "),
                WeightedExampleSum = FindDoubleEntry(lines, "weighted example sum = "),
                WeightedLabelSum = FindDoubleEntry(lines, "weighted label sum = ")
            };

            return stats;
        }

        private static double? FindAverageLossEntry(string[] lines)
        {
            var label = "average loss = ";
            var candidate = lines.FirstOrDefault(l => l.StartsWith(label));

            if (candidate == null)
            {
                return null;
            }

            candidate = candidate.Substring(label.Length);
            if (candidate.EndsWith(" h"))
            {
                candidate = candidate.Substring(0, candidate.Length - 2);
            }

            var ret = 0.0;
            if (double.TryParse(candidate, NumberStyles.Float, CultureInfo.InvariantCulture, out ret))
            {
                return ret;
            }

            return null;
        }

        private static double? FindDoubleEntry(string[] lines, string label)
        {
            var candidate = lines.FirstOrDefault(l => l.StartsWith(label));

            if (candidate == null)
            {
                return null;
            }

            var ret = 0.0;
            if (double.TryParse(candidate.Substring(label.Length), NumberStyles.Float, CultureInfo.InvariantCulture, out ret))
            {
                return ret;
            }

            return null;
        }

        private static ulong? FindULongEntry(string[] lines, string label)
        {
            var candidate = lines.FirstOrDefault(l => l.StartsWith(label));

            if (candidate == null)
            {
                return null;
            }

            ulong ret = 0L;
            if (ulong.TryParse(candidate.Substring(label.Length), NumberStyles.Float, CultureInfo.InvariantCulture, out ret))
            {
                return ret;
            }

            return null;
        }
    }
}

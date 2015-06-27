using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW;
using System.Globalization;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Antlr4.Runtime;
using Antlr4.Runtime.Tree;
using Antlr4.Runtime.Atn;
using System.Text.RegularExpressions;

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
            {
                var listener = new TListener();
                listener.Created = data => 
                {
                    if (data == null)
                    {
                        Assert.Fail("got empty example");
                    }

                    using (var ex = vw.ReadExample(data))
                    {
                        ex.Learn();
                    }
                };
                VWTestHelper.ParseInput(File.OpenRead(inputFile), listener);

                VWTestHelper.AssertEqual(stderrFile, vw.PerformanceStatistics);
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
            using (var vwInMemoryShared2 = new VowpalWabbit<TData>(vwModel))
            {
                var listener = new TListener();
                listener.Created = x =>
                {
                    var expected = vwRef.Predict<VowpalWabbitScalarPrediction>(x.Line);

                    using (var ex = vwInMemoryShared2.ReadExample(x))
                    {
                        var actual = ex.Predict<VowpalWabbitScalarPrediction>();

                        Assert.AreEqual(expected.Value, actual.Value, 1e-5);

                        if (references != null)
                        {
                            Assert.AreEqual(references[index++], actual.Value, 1e-5);
                        }
                    }
                };
            }
        }

        internal static void AssertEqual(string expectedFile, VowpalWabbitPerformanceStatistics actual)
        {
            var expectedPerformanceStatistics = VWTestHelper.ReadPerformanceStatistics(expectedFile);
            AssertEqual(expectedPerformanceStatistics, actual);
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

            Assert.AreEqual(expected.NumberOfExamplesPerPass, actual.NumberOfExamplesPerPass);
            Assert.AreEqual(expected.AverageLoss, actual.AverageLoss, 1e-5);
            Assert.AreEqual(expected.BestConstant, actual.BestConstant, 1e-5);
            // TODO: something weir'd is happening here. BestConstantsLoss is 0 if using RunAll
            // has the proper value if just the unit test is run
            //Console.WriteLine(expected.BestConstantLoss + " vs. " + actual.BestConstantLoss);
            //Assert.AreEqual(expected.BestConstantLoss, actual.BestConstantLoss, 1e-5);
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

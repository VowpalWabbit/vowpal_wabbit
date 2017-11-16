using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW;

namespace cs_unittest
{
    public static class RunTestsHelper
    {
        private static StreamReader Open(string input)
        {
            if (input.EndsWith(".gz"))
                return new StreamReader(new GZipStream(new FileStream(input, FileMode.Open), CompressionMode.Decompress));
            else
                return new StreamReader(input);
        }

        private static bool IsMultilineData(string input)
        {
            using (var streamReader = Open(input))
            {
                string dataLine;
                while ((dataLine = streamReader.ReadLine()) != null)
                {
                    if (string.IsNullOrWhiteSpace(dataLine))
                    {
                        return true;
                    }
                }
            }

            return false;
        }

        public static void ExecuteTest(int testCaseNr, string args, string input, string stderr, string predictFile)
        {
            using (var vw = new VowpalWabbit(args))
            {
                var multiline = IsMultilineData(input);
                using (var streamReader = Open(input))
                {
                    if (multiline)
                    {
                        var lines = new List<string>();

                        string dataLine;
                        while ((dataLine = streamReader.ReadLine()) != null)
                        {
                            if (string.IsNullOrWhiteSpace(dataLine))
                            {
                                if (lines.Count > 0)
                                {
                                    if (args.Contains("-t")) // test only
                                        vw.Predict(lines);
                                    else
                                        vw.Learn(lines);
                                }

                                lines.Clear();
                                continue;
                            }

                            lines.Add(dataLine);
                        }
                    }
                    else
                    {
                        int lineNr = 0;
                        string[] predictions = null;
                        if (File.Exists(predictFile))
                            predictions = File.ReadAllLines(predictFile);

                        string dataLine;
                        while ((dataLine = streamReader.ReadLine()) != null)
                        {
                            if (!string.IsNullOrWhiteSpace(predictFile) && File.Exists(predictFile))
                            {
                                object actualValue;
                                if (args.Contains("-t")) // test only
                                    actualValue = vw.Predict(dataLine, VowpalWabbitPredictionType.Dynamic);
                                else
                                    actualValue = vw.Learn(dataLine, VowpalWabbitPredictionType.Dynamic);

                                if (predictions != null)
                                {
                                    // validate predictions
                                    var actualFloat = actualValue as float?;
                                    if (actualFloat != null)
                                    {
                                        var expectedPrediction = float.Parse(predictions[lineNr].Split(' ').First(), CultureInfo.InvariantCulture);
                                        VWTestHelper.FuzzyEqual(expectedPrediction, (float)actualFloat, 1e-4, "Prediction mismatch");
                                    }

                                    var actualScalar = actualValue as VowpalWabbitScalar?;
                                    if (actualScalar != null)
                                    {
                                        var expectedPredictions = predictions[lineNr]
                                            .Split(' ')
                                            .Select(field => float.Parse(field, CultureInfo.InvariantCulture))
                                            .ToArray();

                                        Assert.AreEqual(2, expectedPredictions.Length);
                                        VWTestHelper.FuzzyEqual(expectedPredictions[0], actualScalar.Value.Value, 1e-4, "Prediction value mismatch");
                                        VWTestHelper.FuzzyEqual(expectedPredictions[1], actualScalar.Value.Confidence, 1e-4, "Prediction confidence mismatch");
                                    }
                                }
                            }
                            else
                                vw.Learn(dataLine);

                            lineNr++;
                        }
                    }

                    if (vw.Arguments.NumPasses > 1)
                        vw.RunMultiPass();
                    else
                        vw.EndOfPass();

                    if (!string.IsNullOrWhiteSpace(stderr) && File.Exists(stderr))
                        VWTestHelper.AssertEqual(stderr, vw.PerformanceStatistics);
                }
            }
        }
    }
}

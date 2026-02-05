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
        public static StreamReader Open(string input)
        {
            if (string.IsNullOrEmpty(input))
            {
                // if filename is not given, return an empty stream
                // this is used for compatibility with test cases that have no filename
                return new StreamReader(new MemoryStream());
            }

            if (input.EndsWith(".gz"))
                return new StreamReader(new GZipStream(new FileStream(input, FileMode.Open), CompressionMode.Decompress));
            else
                return new StreamReader(input);
        }

        private static bool IsMultilineLearner(string args)
        {
            // These learners require multiline input (multi_ex) regardless of data format
            // NOTE: --cbify (not --cbify_ldf) and --warm_cb wrap cb_adf but are themselves singleline
            if ((args.Contains("--cbify") && !args.Contains("--cbify_ldf")) || args.Contains("--warm_cb"))
                return false;

            return args.Contains("--cb_adf") ||
                   args.Contains("--cb_explore_adf") ||
                   args.Contains("--ccb_explore_adf") ||
                   args.Contains("--slates") ||
                   args.Contains("--cbify_ldf") ||
                   args.Contains("--explore_eval") ||
                   args.Contains("--search_task") ||
                   args.Contains("--search ") ||
                   args.Contains("--search=") ||
                   args.Contains("--csoaa_ldf") ||
                   args.Contains("--wap_ldf");
        }

        private static bool IsSinglelineLearner(string args)
        {
            // These learners are explicitly singleline even with LDF-formatted data
            // They process each line individually, blank lines are just separators
            return (args.Contains("--cbify") && !args.Contains("--cbify_ldf")) ||
                   args.Contains("--warm_cb") ||
                   (args.Contains("--csoaa ") || args.Contains("--csoaa=")) && !args.Contains("--csoaa_ldf") ||
                   (args.Contains("--wap ") || args.Contains("--wap=")) && !args.Contains("--wap_ldf");
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

        private static bool RequiresMultilineInput(string args, string input)
        {
            // If args explicitly specify a multiline learner, use multiline
            if (IsMultilineLearner(args))
                return true;

            // If args explicitly specify a singleline learner, use singleline
            if (IsSinglelineLearner(args))
                return false;

            // For test-only mode with loaded models (-i flag), check the data format
            // because the model's learner type isn't visible from command line
            if (args.Contains("-i ") || args.Contains("--initial_regressor"))
                return IsMultilineData(input);

            // Default: VW's default learner (gd/sgd) is singleline
            // Empty lines in data should be skipped, not treated as multiline separators
            return false;
        }

        private static bool IsCsvInput(string args)
        {
            return args.Contains("--csv");
        }

        private static bool HasCsvHeader(string args)
        {
            // CSV files have headers unless --csv_no_file_header is specified
            return IsCsvInput(args) && !args.Contains("--csv_no_file_header");
        }

        private static string StripShellOnlyOptions(string args)
        {
            // These options are only valid for the VW command-line shell, not the library API
            // --onethread: controls shell's thread pool (library doesn't have one)
            return args.Replace("--onethread", "").Replace("  ", " ").Trim();
        }

        public static void ExecuteTest(int testCaseNr, string args, string input, string stderr, string predictFile)
        {
            // Track if --onethread was specified (affects SVRG and other algorithms)
            var useOneThread = args.Contains("--onethread");

            // Strip shell-only options that aren't recognized by the library
            args = StripShellOnlyOptions(args);

            var isJson = args.Contains("--json") || args.Contains("--dsjson");
            if (isJson)
            {
                ExecuteTestJson(testCaseNr, args, input, stderr, predictFile);
                return;
            }

            // CSV input is handled natively by VW - use Driver() instead of line-by-line
            if (IsCsvInput(args))
            {
                ExecuteTestWithDriver(testCaseNr, args, input, stderr, predictFile, useOneThread);
                return;
            }

            // Multi-pass learning (e.g., SVRG, BFGS) requires the parser to re-read from cache
            // Line-by-line processing doesn't work - must use Driver()
            // Note: use "-c " with space to avoid matching --csoaa which contains "-c" as substring
            if (args.Contains("--passes") || args.Contains("-c ") || args.Contains("--cache"))
            {
                ExecuteTestWithDriver(testCaseNr, args, input, stderr, predictFile, useOneThread);
                return;
            }

            using (var vw = new VowpalWabbit(args))
            {
                // Determine if multiline input is needed based on learner type
                // For explicit learner args, use the learner type; for loaded models, check data format
                var multiline = RequiresMultilineInput(args, input);
                using (var streamReader = Open(input))
                {
                    if (multiline)
                    {
                        var lines = new List<string>();

                        string dataLine;
                        do
                        {
                            dataLine = streamReader.ReadLine();
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
                        } while (dataLine != null);
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
                            // Skip empty lines in singleline mode
                            if (string.IsNullOrWhiteSpace(dataLine))
                                continue;

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
                            {
                                vw.Learn(dataLine);
                            }
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

        public static void ExecuteTestWithDriver(int testCaseNr, string args, string input, string stderr, string predictFile, bool useOneThread = false)
        {
            // Use VW's native Driver() for formats like CSV that need native parsing
            using (var vw = new VowpalWabbit(args))
            {
                if (useOneThread)
                    vw.DriverOneThread();
                else
                    vw.Driver();

                if (!string.IsNullOrWhiteSpace(stderr) && File.Exists(stderr))
                    VWTestHelper.AssertEqual(stderr, vw.PerformanceStatistics);
            }
        }

        public static void ExecuteTestJson(int testCaseNr, string args, string input, string stderr, string predictFile)
        {
            using (var vwj = new VowpalWabbitJson(args))
            {
                using (var streamReader = Open(input))
                {
                    int lineNr = 0;
                    string[] predictions = null;
                    if (File.Exists(predictFile))
                        predictions = File.ReadAllLines(predictFile);

                    string dataLine;
                    while ((dataLine = streamReader.ReadLine()) != null)
                    {
                        if (string.IsNullOrWhiteSpace(dataLine))
                            continue;

                        if (!string.IsNullOrWhiteSpace(predictFile) && File.Exists(predictFile))
                        {
                            object actualValue;
                            if (args.Contains("-t")) // test only
                                actualValue = vwj.Predict(dataLine, VowpalWabbitPredictionType.Dynamic);
                            else
                                actualValue = vwj.Learn(dataLine, VowpalWabbitPredictionType.Dynamic);

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
                        {
                            vwj.Learn(dataLine);
                        }
                        lineNr++;
                    }

                    if (vwj.Native.Arguments.NumPasses > 1)
                        vwj.Native.RunMultiPass();
                    else
                        vwj.Native.EndOfPass();

                    if (!string.IsNullOrWhiteSpace(stderr) && File.Exists(stderr))
                        VWTestHelper.AssertEqual(stderr, vwj.Native.PerformanceStatistics);
                }
            }
        }
    }
}

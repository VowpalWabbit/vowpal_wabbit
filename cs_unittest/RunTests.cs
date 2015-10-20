using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using VW;

namespace cs_unittest
{
    [TestClass]
    public class RunTests : TestBase
    {
        private string comment = string.Empty;
        private string stdout = string.Empty;
        private string stderr = string.Empty;
        private string predict = string.Empty;
        private string args = string.Empty;
        private string nr = string.Empty;
        private bool skipTest = false;

        private void Reset()
        {
            comment = stdout = stderr = predict = args = nr = string.Empty;
            skipTest = false;
        }

        [TestMethod]
        [DeploymentItem("RunTests")]
        [DeploymentItem("train-sets", "train-sets")]
        [DeploymentItem(@"train-sets\ref", @"train-sets\ref")]
        [DeploymentItem(@"test-sets\ref", @"test-sets\ref")]
        [DeploymentItem(@"pred-sets\ref", @"pred-sets\ref")]
        [Ignore]
        public void RunAllTest()
        {
            var runTests = File.ReadAllLines("RunTests")
                .SkipWhile(l => l != "__DATA__");

            Match match;

            foreach (var line in runTests)
            {
                if (line.Trim().Length == 0)
                {
                    if (skipTest)
                    {
                        Reset();
                        continue;
                    }

                    // execute test case
                    var argsBuilder = new StringBuilder(args);

                    var dataFile = ExtractArgument(argsBuilder, @"-d\s+(\S+)");
                    var testing = false;

                    if (dataFile == null)
                    {
                        dataFile = ExtractArgument(argsBuilder, @"-t\s+(\S+)");
                        testing = dataFile != null;
                    }

                    if (dataFile == null)
                    {
                        dataFile = ExtractArgument(argsBuilder, @"(\S+)$");
                    }

                    if (dataFile == null)
                    {
                        Console.WriteLine("Skipping test " + nr);
                        Reset();
                        continue;
                    }

                    ExtractArgument(argsBuilder, @"-p\s+(\S+)");

                    var model = ExtractArgument(argsBuilder, @"-f\s+(\S+)");
                    var multiPass = args.Contains("--passes");

                    List<float> expectedPredictions = null;
                    if (File.Exists(predict))
                    {
                        expectedPredictions = File.ReadLines(predict)
                            .Select(l => float.Parse(l.Split(' ')[0], CultureInfo.InvariantCulture))
                            .ToList();
                    }
                    else
                    {
                        if (testing)
                        {
                            Console.WriteLine("Skipping inconsistent test -t without .predict file");
                            Reset();
                            continue;
                        }
                    }

                    Console.WriteLine("Running test {0}: {1} using {2}", nr, comment, argsBuilder);

                    var lineNr = 0;
                    // TODO: check for -p predict
                    // TODO: need to check which prediction label it will be
                    using (var vw = new VowpalWabbit(argsBuilder.ToString()))
                    {
                        foreach (var dataLine in File.ReadLines(dataFile))
                        {
                            if (expectedPredictions != null)
                            {
                                var expectedValue = expectedPredictions[lineNr++];

                                float actualValue;
                                if (testing)
                                {
                                    actualValue = vw.Predict(dataLine, VowpalWabbitPredictionType.Scalar);
                                }
                                else
                                {
                                    actualValue = vw.Learn(dataLine, VowpalWabbitPredictionType.Scalar);
                                }

                                //Assert.AreEqual(
                                //    expectedValue,
                                //    actualValue,
                                //    1e-5,
                                //    string.Format("Test {0}", nr));
                            }
                            else
                            {
                                vw.Learn(dataLine);
                            }
                        }

                        if (multiPass)
                        {
                            vw.RunMultiPass();
                        }

                        if (model != null)
                        {
                            vw.SaveModel(model);
                        }

                        VWTestHelper.AssertEqual(stderr, vw.PerformanceStatistics);
                    }

                    // reset
                    Reset();
                }
                else if ((match = Regex.Match(line, @"^# Test (?<nr>\d+):(?<comment>.*)?$")).Success)
                {
                    nr = match.Groups["nr"].Value;
                    comment = match.Groups["comment"].Value;
                }
                else if ((match = Regex.Match(line, @"^\{VW\} (?<args>.*)$")).Success)
                {
                    args = match.Groups["args"].Value;
                }
                else if (line.EndsWith(".stdout"))
                {
                    stderr = line.Trim();
                }
                else if (line.EndsWith(".stderr"))
                {
                    stderr = line.Trim();
                }
                else if (line.EndsWith(".predict"))
                {
                    predict = line.Trim();
                }
                else if (line.StartsWith("#") && line.Contains("SkipC#"))
                {
                    skipTest = true;
                }
            }
        }

        private static string ExtractArgument(StringBuilder argsBuilder, string regex)
        {
            var match = Regex.Match(argsBuilder.ToString(), regex);

            if (!match.Success)
            {
                return null;
            }

            argsBuilder.Remove(match.Index, match.Length);

            return match.Groups[1].Value;
        }
    }
}

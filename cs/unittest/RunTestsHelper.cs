using System;
using System.Collections.Generic;
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
                        string dataLine;
                        while ((dataLine = streamReader.ReadLine()) != null)
                        {
                            if (!string.IsNullOrWhiteSpace(predictFile) && File.Exists(predictFile))
                            {
                                float actualValue;
                                if (args.Contains("-t")) // test only
                                    actualValue = vw.Predict(dataLine, VowpalWabbitPredictionType.Scalar);
                                else
                                    actualValue = vw.Learn(dataLine, VowpalWabbitPredictionType.Scalar);
                            }
                            else
                                vw.Learn(dataLine);
                        }
                    }


                    if (vw.Arguments.NumPasses > 0)
                        vw.RunMultiPass();

                    if (!string.IsNullOrWhiteSpace(stderr) && File.Exists(stderr))
                        VWTestHelper.AssertEqual(stderr, vw.PerformanceStatistics);
                }
            }
        }
    }
}

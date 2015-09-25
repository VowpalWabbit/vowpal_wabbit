using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using VW;

namespace cs_testhelp
{
    class Program
    {
        static void Main(string[] mainArgs)
        {
            var vwRoot = mainArgs[0];

            var lines = File.ReadAllLines(vwRoot + @"\test\RunTests")
                .SkipWhile(l => l != "__DATA__")
                .ToList();

            var skipList = new[] { 13, 14, 18, 25, 26, 33, 16, 17, 19, 20, 24, 31, 32 };
            var dependencies = new Dictionary<int, int[]>
            {
                { 6, new[] { 4 } },
                { 8, new[] { 7 } },
                { 20, new[] { 19 } },
                { 28, new[] { 27 } },
                { 31, new[] { 30 } },
                { 32, new[] { 30, 31 } }
            };

            var testCode = new Dictionary<int, Tuple<string, string>>();

            using (var cs = new StreamWriter(vwRoot + @"\cs_unittest\TestAll.cs"))
            {
                Environment.CurrentDirectory = vwRoot + @"\test";

                cs.WriteLine(@"
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.IO;
using VW;

namespace cs_unittest
{
	[TestClass]
    public partial class TestAll
    {
");

                Match match;
                for (var i = 0; i < lines.Count; i++)
                {
                    var line = lines[i];

                    if (line.Trim().Length == 0)
                    {
                        if (skipTest)
                        {
                            Reset();
                            continue;
                        }


                        VowpalWabbitArguments arguments;
                        try
                        {
                            using (var vw = new VowpalWabbit(args))
                            {
                                arguments = vw.Arguments;
                            }
                        }
                        catch (Exception e)
                        {
                            cs.WriteLine("// Unable to parse command line: " + e.Message);
                            Reset();
                            continue;
                        }

                        List<float> expectedPredictions = null;
                        if (File.Exists(predict))
                        {
                            try
                            {
                                expectedPredictions = File.ReadLines(predict)
                                    .Select(l => float.Parse(l.Split(' ')[0], CultureInfo.InvariantCulture))
                                    .ToList();
                            }
                            catch (Exception)
                            {
                                cs.WriteLine("// Skipping test, unable to parse prediction file");
                                Reset();
                                continue;
                            }
                        }
                        else
                        {
                            if (arguments.TestOnly)
                            {
                                cs.WriteLine("// Skipping inconsistent test -t without .predict file");
                                Reset();
                                continue;
                            }
                        }

                        string dataDirectory;

                        try
                        {
                            dataDirectory = Path.GetDirectoryName(arguments.Data);
                        }
                        catch (Exception)
                        {
                            cs.WriteLine("// failed to parse <#=args#>. Invalid data file: " + arguments.Data);
                            Reset();
                            continue;
                        }

                        cs.WriteLine(@"
		[TestMethod]
		[Description(""{0}"")]
		[TestCategory(""Command line"")]",
                        comment.Trim().Replace("\"", "\\\""));

                        int[] tests;
                        if (!dependencies.TryGetValue(nr, out tests))
                        {
                            tests = new int[0];
                        }

                        // header deps
                        foreach (var t in tests)
                            cs.WriteLine(testCode[t].Item1);

                        var header = new StringWriter();
                        header.WriteLine("[DeploymentItem(@\"{0}\",@\"{1}\")]",
                            arguments.Data, dataDirectory);
                        header.WriteLine("[DeploymentItem(@\"{0}\", @\"{1}\")]",
                            stderr, Path.GetDirectoryName(stderr));
                        cs.WriteLine(header.ToString());

                        cs.WriteLine("public void CommandLine_Test{0}() {{", nr);

                        // code deps
                        foreach (var t in tests)
		                    cs.WriteLine(testCode[t].Item2);

                        var sw = new StringWriter();
                        sw.WriteLine(@"
			using (var vw = new VowpalWabbit(""{0}""))
            {{
				foreach (var dataLine in File.ReadLines(""{1}""))
				{{",
                            args, arguments.Data);

                        if (expectedPredictions != null)
                        {
                            // cs.WriteLine("var expectedValue = expectedPredictions[lineNr++];");

                            if (arguments.TestOnly)
                                sw.WriteLine("var actualValue = vw.Predict(dataLine, VowpalWabbitPredictionType.Scalar);");
                            else
                                sw.WriteLine("var actualValue = vw.Learn(dataLine, VowpalWabbitPredictionType.Scalar);");
                        }
                        else
                        {
                            sw.WriteLine("vw.Learn(dataLine);");
                        }

                        sw.WriteLine("}");
                        if (arguments.NumPasses > 0)
                            sw.WriteLine("vw.RunMultiPass();");

                        sw.WriteLine("VWTestHelper.AssertEqual(\"{0}\", vw.PerformanceStatistics);",
                            stderr);

                        sw.WriteLine("}");

                        testCode.Add(nr, Tuple.Create(header.ToString(), sw.ToString()));

                        Reset();

                        cs.WriteLine(sw.ToString());
                        cs.WriteLine("}");
                    }
                    else if ((match = Regex.Match(line, @"^#\s*Test\s+(?<nr>\d+):(?<comment>.*)?$")).Success)
                    {
                        nr = int.Parse(match.Groups["nr"].Value);
                        comment = match.Groups["comment"].Value;

                        if (nr > 40 || skipList.Contains(nr))
                        {
                            skipTest = true;
                        }
                    }
                    else if ((match = Regex.Match(line, @"^\{VW\} (?<args>.*)$")).Success)
                    {
                        args = match.Groups["args"].Value;

                        while (args.EndsWith("\\"))
                        {
                            args = args.Substring(0, args.Length - 1);
                            args = args.Trim() + " " + lines[++i].Trim();
                        }
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

                cs.WriteLine("} }");

            }
        }

        static string comment = string.Empty;
        static string stdout = string.Empty;
        static string stderr = string.Empty;
        static string predict = string.Empty;
        static string args = string.Empty;
        static int nr = 0;
        static bool skipTest = false;

        private static void Reset()
        {
            comment = stdout = stderr = predict = args = string.Empty;
            nr = 0;
            skipTest = false;
        }
    }
}

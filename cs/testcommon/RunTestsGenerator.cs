using System;
using System.Collections.Generic;
using System.Linq;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;

using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.Text;
using Newtonsoft.Json;

namespace Vw.Net.Test
{
  [Generator]
  public class RunTestsGenerator : ISourceGenerator
  {
    // static IEnumerable<(CsvLoadType, bool, AdditionalText)> GetLoadOptions(GeneratorExecutionContext context)
    //     {
    //         foreach (AdditionalText file in context.AdditionalFiles)
    //         {
    //             if (Path.GetExtension(file.Path).Equals(".csv", StringComparison.OrdinalIgnoreCase))
    //             {
    //                 // are there any options for it?
    //                 context.AnalyzerConfigOptions.GetOptions(file).TryGetValue("build_metadata.additionalfiles.CsvLoadType", out string? loadTimeString);
    //                 Enum.TryParse(loadTimeString, ignoreCase: true, out CsvLoadType loadType);

    //                 context.AnalyzerConfigOptions.GetOptions(file).TryGetValue("build_metadata.additionalfiles.CacheObjects", out string? cacheObjectsString);
    //                 bool.TryParse(cacheObjectsString, out bool cacheObjects);

    //                 yield return (loadType, cacheObjects, file);
    //             }
    //         }
    //     }

    private bool IsRunTestsFile(AdditionalText file)
    {
      return file.Path.EndsWith("vwtest.json");
    }

    private static readonly HashSet<int> SkipList = new HashSet<int>(new [] {
        // Model dependency (depends_on other tests, C# test generator doesn't handle dependencies)
        31, 32, 33, 34, 412,
        // --examples flag not respected by C# test helper
        69,
        // Reference file mismatch (CLI now emits warnings not in reference)
        193,
        // cats/cats_pdf uses ACTION_PDF_VALUE prediction type not supported in C# wrapper
        225, 227, 309, 345, 346,
        // flatbuffer input not supported in C# wrapper
        237, 238, 239, 240, 241, 242, 243, 244,
        // --help causes process exit, crashing test host
        273, 274,
    });

    private bool ShouldInclude(RunTestEntry entry)
    {
      return entry.vw_command != null && !entry.vw_command.Contains("--dsjson") && !entry.desc.Contains("SkipC#") && !entry.skip_csharp;
    }

    private string MatchArgument(string args, string option)
    {
      Match match = Regex.Match(args, Regex.Escape(option) + @"\s+(?<value>\S+)");
      return match.Success ? match.Groups["value"].Value : "";
    }

    private TestCase GenerateTestCase(RunTestEntry entry, Dictionary<string, TestCase> outputModels, string testRoot)
    {
      TestCase testCase = new TestCase()
      {
        Id = entry.id,
        Comment = entry.desc.Replace("\"", "\"\""),
        Arguments = entry.vw_command,
        InputData = MatchArgument(entry.vw_command, "-d"),
        InitialRegressor = MatchArgument(entry.vw_command, "-i"),
        FinalRegressor = MatchArgument(entry.vw_command, "-f")
      };

      foreach (KeyValuePair<string, string> diffFile in entry.diff_files)
      {
        if (diffFile.Key == "stderr")
        {
          testCase.Stderr = diffFile.Value;
        }
        else if (diffFile.Key.EndsWith(".predict"))
        {
          testCase.Predict = diffFile.Value;
        }
      }
      
      // resolve dependencies
      if (!string.IsNullOrEmpty(testCase.FinalRegressor))
        outputModels[testCase.FinalRegressor] = testCase;

      if (!string.IsNullOrEmpty(testCase.InitialRegressor))
      {
        TestCase dep;
        bool foundInitialRegressor = false;
        if (outputModels.TryGetValue(testCase.InitialRegressor, out dep))
        {
            testCase.Dependency = dep;
            foundInitialRegressor = true;
        }
        else if (testCase.InitialRegressor.StartsWith("model-sets/"))
        {
            string fullPath = Path.Combine(testRoot, testCase.InitialRegressor);
            foundInitialRegressor = File.Exists(fullPath);
        }

        if (!foundInitialRegressor)
        {
            throw new Exception("Missing dependency: '" + testCase.InitialRegressor + "' for test case " + testCase.Id);
        }
      }

      return testCase;
    }

    private string GenerateSource(string testSet, Dictionary<int, TestCase> testCases)
    {
      StringBuilder sb = new StringBuilder();
      sb.Append(
$@"
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.IO;
using System.IO.Compression;
using VW;

namespace cs_unittest 
{{
  [TestClass]
  public partial class RunTests_{testSet} : TestBase
  {{
");

      foreach (int test in testCases.Keys)
      {
        TestCase mainTestCase = testCases[test];
        sb.Append(
$@"
    [TestMethod]
    [Description(@""{ mainTestCase.Comment?.Trim() ?? string.Empty }"")]
"
        );

        if (SkipList.Contains(test))
        {
          sb.AppendLine("    [Ignore]");
        }

        sb.Append(
$@"
    [TestCategory(""Vowpal Wabbit/Command Line"")]
    public void CommandLine_Test{test}()
    {{
"
        );

        foreach (TestCase testCase in mainTestCase.InDependencyOrder())
        {
          sb.Append(
$@"
      RunTestsHelper.ExecuteTest(
        {testCase.Id}, 
        ""{testCase.Arguments}"", 
        ""{testCase.InputData}"", 
        ""{testCase.Stderr}"", 
        ""{testCase.Predict}"");
"
          );
        }

        sb.AppendLine("    }");
      }

      sb.Append(
$@"
  }}
}}");

      return sb.ToString();
    }

    public void Execute(GeneratorExecutionContext context)
    {
      foreach (AdditionalText file in context.AdditionalFiles.Where(IsRunTestsFile))
      {
        string testSet = Path.GetFileNameWithoutExtension(file.Path).Replace(".", "_");
        string targetFile = testSet + ".Generated.cs";
        string testRoot = Path.GetDirectoryName(file.Path);

        List<RunTestEntry> entries = JsonConvert.DeserializeObject<List<RunTestEntry>>(file.GetText().ToString());

        Dictionary<string, TestCase> outputModels = new Dictionary<string, TestCase>();
        Dictionary<int, TestCase> testCases = entries.Where(ShouldInclude)
                                                     .Select(e => GenerateTestCase(e, outputModels, testRoot))
                                                     .ToDictionary(t => t.Id);

        string source = GenerateSource(testSet, testCases);
        
        context.AddSource(targetFile, source);
      }
    }

    public void Initialize(GeneratorInitializationContext context)
    {
      // No initialization required for this one
    }
  }

  public class RunTestEntry
  {
    public int id { get; set; }
    public string desc { get; set; }
    public string vw_command { get; set; }
    public string bash_command { get; set; }
    public Dictionary<string, string> diff_files { get; set; }
    public IList<string> input_files { get; set; }
    public IList<int> depends_on { get; set; }
    public bool skip_csharp { get; set; }
  }

  internal class TestCase
  {
    public int Id;

    public string Arguments = "";

    public string InitialRegressor;

    public string FinalRegressor;

    public string InputData = "";

    public string Stderr = "";

    public string Predict = "";

    public string Comment;

    public TestCase Dependency;

    public List<TestCase> InDependencyOrder()
    {
        var tests = new List<TestCase>();

        var dep = this;
        while (dep != null)
        {
            tests.Add(dep);
            dep = dep.Dependency;
        }

        tests.Reverse();
        return tests;
    }
  }
}
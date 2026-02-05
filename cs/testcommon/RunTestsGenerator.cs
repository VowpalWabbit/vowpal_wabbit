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
        // --examples flag not respected by C# test helper
        69,
        // flatbuffer input not supported in C# wrapper
        237, 238, 239, 240, 241, 242, 243, 244,
        // --help causes process exit, crashing test host
        273, 274,
    });

    private bool ShouldInclude(RunTestEntry entry)
    {
      return entry.vw_command != null && !entry.vw_command.Contains("--dsjson") && !entry.desc.Contains("SkipC#");
    }

    private string MatchArgument(string args, string option)
    {
      Match match = Regex.Match(args, Regex.Escape(option) + @"\s+(?<value>\S+)");
      return match.Success ? match.Groups["value"].Value : "";
    }

    private TestCase GenerateTestCase(RunTestEntry entry)
    {
      TestCase testCase = new TestCase()
      {
        Id = entry.id,
        Comment = entry.desc.Replace("\"", "\"\""),
        Arguments = entry.vw_command,
        InputData = MatchArgument(entry.vw_command, "-d"),
        InitialRegressor = MatchArgument(entry.vw_command, "-i"),
        FinalRegressor = MatchArgument(entry.vw_command, "-f"),
        DependsOn = entry.depends_on
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

      return testCase;
    }

    private void ResolveDependencies(Dictionary<int, TestCase> testCases)
    {
      foreach (var testCase in testCases.Values)
      {
        // Use explicit depends_on from JSON if available
        if (testCase.DependsOn != null && testCase.DependsOn.Count > 0)
        {
          // For simplicity, use the last dependency in the chain
          // (tests typically have a single dependency or a linear chain)
          foreach (int depId in testCase.DependsOn)
          {
            if (testCases.TryGetValue(depId, out TestCase dep))
            {
              testCase.Dependency = dep;
            }
          }
        }
      }
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

        List<RunTestEntry> entries = JsonConvert.DeserializeObject<List<RunTestEntry>>(file.GetText().ToString());

        // First pass: create all test cases
        Dictionary<int, TestCase> testCases = entries.Where(ShouldInclude)
                                                     .Select(e => GenerateTestCase(e))
                                                     .ToDictionary(t => t.Id);

        // Second pass: resolve dependencies using depends_on field
        ResolveDependencies(testCases);

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

    public IList<int> DependsOn;

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
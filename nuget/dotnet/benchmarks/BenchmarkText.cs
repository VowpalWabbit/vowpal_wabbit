using VW;
using BenchmarkDotNet.Attributes;

[Config(typeof(VWBenchmarkConfig))]
public class BenchmarkText
{
    [Benchmark]
    [ArgumentsSource(nameof(ArgsGenerator))]
    public void Benchmark(Args args)
    {
        var ex = args.vw.ParseLine(args.exampleStr);
        ex.Dispose();
    }

    public IEnumerable<Args> ArgsGenerator()
    {
        yield return new Args("120_string_fts", Common.GetStringFeatures(120));
        yield return new Args("120_num_features", Common.GetNumericalFeatures(120));
    }

    public class Args
    {
        public string testCaseName;
        public VowpalWabbit vw;
        public string exampleStr;

        public Args(string test_case_name, string example_str)
        {
            testCaseName = test_case_name;
            vw = new VowpalWabbit("--cb 2 --quiet");
            exampleStr = example_str;
        }

        public override string ToString()
        {
            return testCaseName;
        }
    }
}


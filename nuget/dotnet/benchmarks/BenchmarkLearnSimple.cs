using VW;
using BenchmarkDotNet.Attributes;

[Config(typeof(VWBenchmarkConfig))]
public class BenchmarkLearnSimple
{
    [Benchmark]
    [ArgumentsSource(nameof(ArgsGenerator))]
    public void Benchmark(Args args)
    {
        args.vw.Learn(args.example);
    }

    public IEnumerable<Args> ArgsGenerator()
    {
        yield return new Args("8_features", "1 zebra|MetricFeatures:3.28 height:1.5 length:2.0 |Says black with white stripes |OtherFeatures NumberOfLegs:4.0 HasStripes");
        yield return new Args("1_feature", "1 | a");
    }

    public class Args
    {
        public string testCaseName;
        public VowpalWabbit vw;
        public VowpalWabbitExample example;

        public Args(string name, string example_str)
        {
            testCaseName = name;
            vw = new VowpalWabbit("--quiet");
            example = vw.ParseLine(example_str);
        }

        public override string ToString()
        {
            return testCaseName;
        }
    }
}


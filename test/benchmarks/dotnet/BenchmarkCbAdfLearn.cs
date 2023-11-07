using VW;
using BenchmarkDotNet.Attributes;

[Config(typeof(VWBenchmarkConfig))]
public class BenchmarkCbAdfLearn
{
    [Benchmark]
    [ArgumentsSource(nameof(ArgsGenerator))]
    public void Benchmark(Args args)
    {
        args.vw.Learn(args.examples);
    }

    public IEnumerable<Args> ArgsGenerator()
    {
        yield return new Args("few_features", 2);
        yield return new Args("many_features", 120);
    }

    public class Args
    {
        public string testCaseName;
        public VowpalWabbit vw;
        public List<VowpalWabbitExample> examples;

        public Args(string name, int feature_count)
        {
            testCaseName = name;
            vw = new VowpalWabbit("--cb_explore_adf --epsilon 0.1 --quiet -q ::");
            examples = new List<VowpalWabbitExample>();
            examples.Add(vw.ParseLine("shared tag1| s_1 s_2"));
            examples.Add(vw.ParseLine(Common.GetStringFeatures(feature_count)));
            examples.Add(vw.ParseLine(Common.GetStringFeaturesNoLabel(feature_count)));
            examples.Add(vw.ParseLine(Common.GetStringFeaturesNoLabel(feature_count)));
        }

        public override string ToString()
        {
            return testCaseName;
        }
    }
}


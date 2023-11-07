using VW;
using BenchmarkDotNet.Attributes;

[Config(typeof(VWBenchmarkConfig))]
public class BenchmarkCcbAdfLearn
{
    [Benchmark]
    [ArgumentsSource(nameof(ArgsGenerator))]
    public void Benchmark(Args args)
    {
        args.vw.Learn(args.examples);
    }

    public IEnumerable<Args> ArgsGenerator()
    {
        yield return new Args("few_features", "a");
        yield return new Args("many_features", "a b c d e f g h i j k l m n o p q r s t u v w x y z");

        // --no-predict not supported in C#
        // yield return new Args("few_features_no_predict", "a", " --no-predict");
        // yield return new Args("many_features_no_predict", "a b c d e f g h i j k l m n o p q r s t u v w x y z", " --no-predict");
    }

    public class Args
    {
        public string testCaseName;
        public VowpalWabbit vw;
        public List<VowpalWabbitExample> examples;

        public Args(string name, string feature_string, string cmd = "")
        {
            testCaseName = name;
            vw = new VowpalWabbit("--ccb_explore_adf --quiet" + cmd);
            examples = new List<VowpalWabbitExample>();
            examples.Add(vw.ParseLine("ccb shared |User " + feature_string));
            examples.Add(vw.ParseLine("ccb action |Action1 " + feature_string));
            examples.Add(vw.ParseLine("ccb action |Action2 " + feature_string));
            examples.Add(vw.ParseLine("ccb action |Action3 " + feature_string));
            examples.Add(vw.ParseLine("ccb action |Action4 " + feature_string));
            examples.Add(vw.ParseLine("ccb action |Action5 " + feature_string));
            examples.Add(vw.ParseLine("ccb slot 0:0:0.2 |Slot h"));
            examples.Add(vw.ParseLine("ccb slot 1:0:0.25 |Slot i"));
            examples.Add(vw.ParseLine("ccb slot 2:0:0.333333 |Slot j"));
        }

        public override string ToString()
        {
            return testCaseName;
        }
    }
}


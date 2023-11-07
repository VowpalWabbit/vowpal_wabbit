using System.Text;
using VW;
using BenchmarkDotNet.Attributes;

[Config(typeof(VWBenchmarkConfig))]
public class BenchmarkMulti
{
    [Benchmark]
    [ArgumentsSource(nameof(ArgsGenerator))]
    public void Benchmark(Args args)
    {
        foreach (var examples in args.examples_vec)
        {
            args.vw.Learn(examples);
        }
    }

    public IEnumerable<Args> ArgsGenerator()
    {
        yield return new Args("cb_adf_no_namespaces", GenerateCbExamples(100, 7, 3, 6, 1, 4, 14, 2, false), "--cb_explore_adf --quiet");
        yield return new Args("cb_adf_diff_char_no_interactions", GenerateCbExamples(100, 7, 3, 6, 1, 4, 14, 2, false), "--cb_explore_adf --quiet");
        yield return new Args("cb_adf_diff_char_interactions", GenerateCbExamples(100, 7, 3, 6, 1, 4, 14, 2, false), "--cb_explore_adf --quiet -q ::");
        yield return new Args("cb_adf_same_char_no_interactions", GenerateCbExamples(100, 7, 3, 6, 1, 4, 14, 2, true), "--cb_explore_adf --quiet");
        yield return new Args("cb_adf_same_char_interactions", GenerateCbExamples(100, 7, 3, 6, 1, 4, 14, 2, true), "--cb_explore_adf --quiet -q ::");

        yield return new Args("ccb_adf_no_namespaces", GenerateCcbExamples(50, 7, 3, 6, 1, 4, 14, 2, false, 3), "--ccb_explore_adf --quiet");
        yield return new Args("ccb_adf_diff_char_no_interactions", GenerateCcbExamples(50, 7, 3, 6, 3, 4, 14, 2, false, 3), "--ccb_explore_adf --quiet");
        yield return new Args("ccb_adf_diff_char_interactions", GenerateCcbExamples(50, 7, 3, 6, 3, 4, 14, 2, false, 3), "--ccb_explore_adf --quiet -q ::");
        yield return new Args("ccb_adf_same_char_no_interactions", GenerateCcbExamples(50, 7, 3, 6, 3, 4, 14, 2, true, 3), "--ccb_explore_adf --quiet");
        yield return new Args("ccb_adf_same_char_interactions", GenerateCcbExamples(50, 7, 3, 6, 3, 4, 14, 2, true, 3), "--ccb_explore_adf --quiet -q ::");
    }

    public class Args
    {
        public string testCaseName;
        public VowpalWabbit vw;
        public List<List<VowpalWabbitExample>> examples_vec;

        public Args(string name, List<List<string>> examples, string cmd)
        {
            testCaseName = name;
            vw = new VowpalWabbit(cmd);
            examples_vec = new List<List<VowpalWabbitExample>>();

            foreach (var str_list in examples)
            {
                var examples_list = new List<VowpalWabbitExample>();
                foreach (var str in str_list)
                {
                    examples_list.Add(vw.ParseLine(str));
                }
                examples_vec.Add(examples_list);
            }
        }

        public override string ToString()
        {
            return testCaseName;
        }
    }

    public static List<List<string>> GenerateCbExamples(
        int num_examples,
        int shared_feats_size,
        int shared_feats_count,
        int actions_per_example,
        int feature_groups_size,
        int feature_groups_count,
        int action_feats_size,
        int action_feats_count,
        bool same_first_char
    )
    {
        var examples_vec = new List<List<string>>();
        var rand = new Random();

        foreach (int ex in Enumerable.Range(0, num_examples))
        {
            var examples = new List<string>();
            var shared_ss = new StringBuilder();

            shared_ss.Append("shared |");
            foreach (int shared_feat in Enumerable.Range(0, shared_feats_count))
            {
                shared_ss.Append(" ");
                shared_ss.Append(rand.Next(shared_feats_size));
            }
            examples.Add(shared_ss.ToString());

            int action_ind = rand.Next(actions_per_example);
            foreach (int ac in Enumerable.Range(0, actions_per_example))
            {
                var action_ss = new StringBuilder();
                if (ac == action_ind)
                {
                    action_ss.Append(action_ind);
                    action_ss.Append(":1.0:0.5 ");
                }
                foreach (int fg in Enumerable.Range(0, feature_groups_count))
                {
                    action_ss.Append("|");
                    if (same_first_char)
                        action_ss.Append("f");
                    action_ss.Append(Convert.ToChar(65 + rand.Next(feature_groups_size)));
                    action_ss.Append(" ");

                    foreach (int action_feat in Enumerable.Range(0, action_feats_count))
                    {
                        action_ss.Append(rand.Next(action_feats_size));
                        action_ss.Append(" ");
                    }
                }
                examples.Add(action_ss.ToString());
            }
            examples_vec.Add(examples);
        }

        return examples_vec;
    }

    public static List<List<string>> GenerateCcbExamples(
        int num_examples,
        int shared_feats_size,
        int shared_feats_count,
        int actions_per_example,
        int feature_groups_size,
        int feature_groups_count,
        int action_feats_size,
        int action_feats_count,
        bool same_first_char,
        int slots_per_example
    )
    {
        var examples_vec = new List<List<string>>();
        var rand = new Random();

        foreach (int ex in Enumerable.Range(0, num_examples))
        {
            var examples = new List<string>();
            var shared_ss = new StringBuilder();

            shared_ss.Append("ccb shared |");
            foreach (int shared_feat in Enumerable.Range(0, shared_feats_count))
            {
                shared_ss.Append(" ");
                shared_ss.Append(rand.Next(shared_feats_size));
            }
            examples.Add(shared_ss.ToString());

            foreach (int ac in Enumerable.Range(0, actions_per_example))
            {
                var action_ss = new StringBuilder();
                action_ss.Append("ccb action ");
                foreach (int fg in Enumerable.Range(0, feature_groups_count))
                {
                    action_ss.Append("|");
                    if (same_first_char)
                        action_ss.Append("f");
                    action_ss.Append(Convert.ToChar(65 + rand.Next(feature_groups_size)));
                    action_ss.Append(" ");

                    foreach (int action_feat in Enumerable.Range(0, action_feats_count))
                    {
                        action_ss.Append(rand.Next(action_feats_size));
                        action_ss.Append(" ");
                    }
                }
                examples.Add(action_ss.ToString());
            }

            foreach (int slot in Enumerable.Range(0, slots_per_example))
            {
                var slot_ss = new StringBuilder();
                slot_ss.Append("ccb slot ");
                foreach (int fg in Enumerable.Range(0, feature_groups_count))
                {
                    slot_ss.Append(rand.Next(actions_per_example));
                    slot_ss.Append(":0.");
                    slot_ss.Append(rand.Next(10));
                    slot_ss.Append(":0.");
                    slot_ss.Append(rand.Next(10));
                    slot_ss.Append(" |");

                    if (same_first_char)
                        slot_ss.Append("f");
                    slot_ss.Append(Convert.ToChar(65 + rand.Next(feature_groups_size)));
                    slot_ss.Append(" ");

                    foreach (int slot_feat in Enumerable.Range(0, action_feats_count))
                    {
                        slot_ss.Append(rand.Next(action_feats_size));
                        slot_ss.Append(" ");
                    }
                }
                examples.Add(slot_ss.ToString());
            }
            examples_vec.Add(examples);
        }
        return examples_vec;
    }
}


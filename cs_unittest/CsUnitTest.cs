using Microsoft.Research.MachineLearning;
using Microsoft.Research.MachineLearning.Serializer.Attributes;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace cs_test
{
    [TestClass]
    public class CsUnitTest
    {
        [TestMethod]
        public void VwCleanupTest()
        {
            new VowpalWabbit<Test1>("-k -l 20 --initial_t 128000 --power_t 1 -c --passes 8 --invariant --ngram 3 --skips 1 --holdout_off")
                .Dispose();
        }

        [TestMethod]
        public void VwCleanupTestError()
        {
            try
            {
                new VowpalWabbit<Test1>("-k -l 20 --initial_t 128000 --power_t 1 -f models/0001.model -c --passes 8 --invariant --ngram 3 --skips 1 --holdout_off")
                    .Dispose();

                Assert.Fail("Excepted exception not thrown");
            }
            catch (Exception e)
            {
                Assert.IsTrue(e.Message.Contains("No such file or directory"), e.Message);
            }
        }

        [TestMethod]
        // [Ignore]
        [DeploymentItem(@"train-sets\0001.dat", "train-sets")]
        [DeploymentItem(@"pred-sets\ref\0001.predict", @"pred-sets\ref")]
        public void Test1and2()
        {
            var references = File.ReadAllLines(@"pred-sets\ref\0001.predict").Select(l => float.Parse(l, CultureInfo.InvariantCulture)).ToArray();

            var input = new List<Test1>();

            Directory.CreateDirectory("models");

            using (var vwStr = new VowpalWabbit(" -k -l 20 --initial_t 128000 --power_t 1 -c test1and2.str --passes 8 --invariant --ngram 3 --skips 1 --holdout_off"))
            using (var vw = new VowpalWabbit<Test1>(" -k -l 20 --initial_t 128000 --power_t 1 -c test1and2 --passes 8 --invariant --ngram 3 --skips 1 --holdout_off"))
            using (var fr = new StreamReader(@"train-sets\0001.dat"))
            {
                string line;
                for (var lineNr = 1; (line = fr.ReadLine()) != null; lineNr++)
                {
                    var parts = line.Split('|');

                    var features = parts[1]
                                .Split(' ')
                                .Skip(1) // skip namespace label
                                .ToList();

                    var dictFeatures = from f in features
                                       where !f.StartsWith("const")
                                       let t = f.Split(':')
                                       // weight_index, x
                                       select new KeyValuePair<string, float>(t[0], float.Parse(t[1], CultureInfo.InvariantCulture));

                    var data = new Test1()
                    {
                        Constant = float.Parse(features.First(f => f.StartsWith("const")).Substring(6), CultureInfo.InvariantCulture),
                        Features = dictFeatures.ToList(),
                        Line = line
                    };

                    input.Add(data);

                    using (var strExample = vwStr.ReadExample(line))
                    using (var example = vw.ReadExample(data))
                    {
                        example.AddLabel(float.Parse(parts[0].Trim(), CultureInfo.InvariantCulture));

                        var diff = example.Diff(strExample, true);
                        Assert.IsNull(diff, "Found diff for line " + lineNr + ": " + diff);

                        var actual = example.Learn();
                        var expected = strExample.Learn();

                        Assert.AreEqual(expected, actual, 1e-6, "Learn output differs on line: " + lineNr);
                    }
                }

                vwStr.RunMultiPass();
                vw.RunMultiPass();

                vwStr.SaveModel("models/str0001.model");
                vw.SaveModel("models/0001.model");
            }

            Assert.AreEqual(input.Count, references.Length);

            var samePredictions = 0;

            using (var vwStr = new VowpalWabbit("-k -t -i models/str0001.model --invariant"))
            using (var vw = new VowpalWabbit<Test1>("-k -t -i models/0001.model --invariant"))
            {
                for (var i = 0; i < input.Count; i++)
                {
                    using (var strExample = vwStr.ReadExample(input[i].Line))
                    using (var example = vw.ReadExample(input[i]))
                    {
                        var actual = example.Predict();
                        var expected = strExample.Predict();

                        if (actual == expected)
                        {
                            samePredictions++;
                        }

                        Assert.AreEqual(
                            references[i],
                            expected,
                            1e-5,
                            string.Format(CultureInfo.InvariantCulture, "Expected {0} vs. actual {1} at line {2}", references[i], actual, i));                    
                    }
                }
            }

            Assert.IsTrue(samePredictions / (float)input.Count > .9, "Prediction results diverge too much");
        }

        [TestMethod]
        [Ignore]
        [DeploymentItem(@"train-sets\rcv1_cb_eval", "train-sets")]
        public void Test74()
        {
            // 2 1:1:0.5 | tuesday year million short compan vehicl line stat financ commit exchang plan corp subsid credit issu debt pay gold bureau prelimin refin billion telephon time draw basic relat file spokesm reut secur acquir form prospect period interview regist toront resourc barrick ontario qualif bln prospectus convertibl vinc borg arequip
            using (var vw = new VowpalWabbit<Rcv1CbEval>("--cb 2 --eval"))
            using (var fr = new StreamReader(@"train-sets\rcv1_cb_eval"))
            {
                string line;

                while ((line = fr.ReadLine()) != null)
                {
                    var parts = line.Split('|');

                    var data = new Rcv1CbEval()
                    {
                        Words = parts[1].Split(' ')
                    }; 

                    using(var example = vw.ReadExample(data))
                    {
                        example.AddLabel(parts[0]);
                        example.Learn();
                    }
                }
            }
        }
        
        [TestMethod]
        public void Test87()
        {
            using (var vw = new VowpalWabbit<Test87, Test87ADF>("--cb_adf --rank_all"))
            {
                //shared | s_1 s_2
                //0:1.0:0.5 | a_1 b_1 c_1
                //| a_2 b_2 c_2
                //| a_3 b_3 c_3

                //| b_1 c_1 d_1
                //0:0.0:0.5 | b_2 c_2 d_2

                //| a_1 b_1 c_1
                //| a_3 b_3 c_3

                var chosenAction = new Test87ADF { Features = new [] { "a_1","b_1","c_1" } };
                var example = new Test87
                {
                    Shared = new[] { "s_1", "s_2" },
                    ActionDependentFeatures = new[] {
                        chosenAction,
                        new Test87ADF { Features = new [] { "a_2","b_2","c_2" } },
                        new Test87ADF { Features = new [] { "a_3","b_3","c_3" } },
                    }
                };
                
                var result = vw.Learn(example, chosenAction, 1.0f, 0.5f);

                Assert.ReferenceEquals(example.ActionDependentFeatures[0], result[0]);
                Assert.ReferenceEquals(example.ActionDependentFeatures[1], result[1]);
                Assert.ReferenceEquals(example.ActionDependentFeatures[2], result[2]);

                chosenAction = new Test87ADF { Features = new [] { "b_2", "c_2", "d_2" } };
                example = new Test87
                {
                    ActionDependentFeatures = new[] {
                        new Test87ADF { Features = new [] { "b_1","c_1","d_1" } },
                        chosenAction,
                    }
                };
                
                result = vw.Learn(example, chosenAction, 0.0f, 0.5f);
                Assert.ReferenceEquals(example.ActionDependentFeatures[0], result[1]);
                Assert.ReferenceEquals(example.ActionDependentFeatures[1], result[0]);

                example = new Test87
                {
                    ActionDependentFeatures = new[] {
                        new Test87ADF { Features = new [] { "a_1","b_1","c_1" } },
                        new Test87ADF { Features = new [] { "a_3","b_3","c_3" } }
                    }
                };
                result = vw.Predict(example);

                Assert.ReferenceEquals(example.ActionDependentFeatures[0], result[1]);
                Assert.ReferenceEquals(example.ActionDependentFeatures[1], result[0]);
            }
        }
    }

    public class Test87 : IActionDependentFeatureExample<Test87ADF>
    {
        [Feature]
        public string[] Shared { get; set; }

        public IList<Test87ADF> ActionDependentFeatures { get; set; }
    }

    public class Test87ADF
    {
        [Feature]
        public string[] Features { get; set; }

        public override string ToString()
        {
            return string.Join(" ", this.Features);
        }
    }

    public class Test1
    {
        [Feature(FeatureGroup = 'f', Namespace = "eatures", Name = "const", Order = 2)]
        public float Constant { get; set; }

        [Feature(FeatureGroup = 'f', Namespace = "eatures", Order = 1)]
        public IEnumerable<KeyValuePair<string, float>> Features { get; set; }

        public string Line { get; set; }
    }

    public class Rcv1CbEval
    {
        [Feature]
        public string[] Words { get; set; } 
    }
}

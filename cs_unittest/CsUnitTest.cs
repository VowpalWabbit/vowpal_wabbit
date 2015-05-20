using Microsoft.Research.MachineLearning;
using Microsoft.Research.MachineLearning.Serializer.Attributes;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
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
            // 1 |features 13:3.9656971e-02 24:3.4781646e-02 69:4.6296168e-02 85:6.1853945e-02 140:3.2349996e-02 156:1.0290844e-01 175:6.8493910e-02 188:2.8366476e-02 229:7.4871540e-02 230:9.1505975e-02 234:5.4200061e-02 236:4.4855952e-02 238:5.3422898e-02 387:1.4059304e-01 394:7.5131744e-02 433:1.1118756e-01 434:1.2540409e-01 438:6.5452829e-02 465:2.2644201e-01 468:8.5926279e-02 518:1.0214076e-01 534:9.4191484e-02 613:7.0990764e-02 646:8.7701865e-02 660:7.2289191e-02 709:9.0660661e-02 752:1.0580081e-01 757:6.7965068e-02 812:2.2685185e-01 932:6.8250686e-02 1028:4.8203137e-02 1122:1.2381379e-01 1160:1.3038123e-01 1189:7.1542501e-02 1530:9.2655659e-02 1664:6.5160148e-02 1865:8.5823394e-02 2524:1.6407280e-01 2525:1.1528353e-01 2526:9.7131468e-02 2536:5.7415009e-01 2543:1.4978983e-01 2848:1.0446861e-01 3370:9.2423186e-02 3960:1.5554591e-01 7052:1.2632671e-01 16893:1.9762035e-01 24036:3.2674628e-01 24303:2.2660980e-01 const:.01
            // -d train-sets/0001.dat
            // Directory.CreateDirectory("models");
            var references = File.ReadAllLines(@"pred-sets\ref\0001.predict").Select(l => float.Parse(l, CultureInfo.InvariantCulture)).ToArray();

            var input = new List<Test1>();

            Directory.CreateDirectory("models");

            using (var vw = new VowpalWabbit<Test1>("-k -l 20 --initial_t 128000 --power_t 1 -c --passes 8 --invariant --ngram 3 --skips 1 --holdout_off"))
            using (var fr = new StreamReader(@"train-sets\0001.dat"))
            {
                string line;

                while ((line = fr.ReadLine()) != null)
                {
                    var parts = line.Split('|');

                    var features = parts[1]
                                .Split(' ')
                                // skip namespace label
                                .Skip(1)
                                .ToList();

                    var dictFeatures = from f in features
                              where !f.StartsWith("const")
                              let t = f.Split(':')
                              select new 
                              { 
                                  weight_index = t[0],
                                  x = float.Parse(t[1], CultureInfo.InvariantCulture) 
                              };

                    var data = new Test1()
                    {
                        @const = float.Parse(features.First(f => f.StartsWith("const")).Substring(6), CultureInfo.InvariantCulture),
                        Features = dictFeatures.ToDictionary(a => a.weight_index, a => a.x)
                    };

                    input.Add(data);

                    using(var example = vw.ReadExample(data))
                    {
                        // example.AddLabel(float.Parse(parts[0].Trim(), CultureInfo.InvariantCulture));
                        example.AddLabel(parts[0].Trim());
                        var actual = example.Learn();
                    }
                }

                vw.SaveModel("models/0001.model");
            }

            Assert.AreEqual(input.Count, references.Length);

            // -k -t train-sets/0001.dat -i models/0001.model -p 001.predict.tmp --invariant
            using (var vw = new VowpalWabbit<Test1>("-k -t -i models/0001.model --invariant"))
            {
                for (var i = 0; i < input.Count; i++)
                {
                    using (var example = vw.ReadExample(input[i]))
                    {
                        var actual = example.Predict();

                        Assert.AreEqual(
                            references[i],
                            actual,
                            string.Format(CultureInfo.InvariantCulture, "Expected {0} vs. actual {1} at line {2}", references[i], actual, i));                    

                    }
                }
            }
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
        [Feature(FeatureGroup = 'f', Namespace = "eatures")]
        public float @const { get; set; }

        [Feature(FeatureGroup = 'f', Namespace = "eatures")]
        public Dictionary<string, float> Features { get; set; }
    }

    public class Rcv1CbEval
    {
        [Feature]
        public string[] Words { get; set; } 
    }
}

using cs_unittest;
using Microsoft.Research.MachineLearning;
using Microsoft.Research.MachineLearning.Interfaces;
using Microsoft.Research.MachineLearning.Labels;
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
    public class Test1and2Class : TestBase
    {
        [TestMethod]
        [DeploymentItem(@"train-sets\0001.dat", "train-sets")]
        [DeploymentItem(@"train-sets\ref\0001.stderr", @"train-sets\ref")]
        [DeploymentItem(@"test-sets\ref\0001.stderr", @"test-sets\ref")]
        [DeploymentItem(@"pred-sets\ref\0001.predict", @"pred-sets\ref")]
        public void Test1and2()
        {
            var references = File.ReadAllLines(@"pred-sets\ref\0001.predict").Select(l => float.Parse(l, CultureInfo.InvariantCulture)).ToArray();

            var input = new List<Test1>();

            using (var vwStr = new VowpalWabbit(" -k -c test1and2.str --passes 8 -l 20 --power_t 1 --initial_t 128000  --ngram 3 --skips 1 --invariant --holdout_off"))
            using (var vw = new VowpalWabbit<Test1>(" -k -c test1and2 --passes 8 -l 20 --power_t 1 --initial_t 128000  --ngram 3 --skips 1 --invariant --holdout_off"))
            {
                var lineNr = 0;
                VWTestHelper.ParseInput(
                    File.OpenRead(@"train-sets\0001.dat"), 
                    new MyListener(data =>
                    {
                        input.Add(data);

                        var expected = vwStr.Learn<VowpalWabbitScalarPrediction>(data.Line);

                        using (var example = vw.ReadExample(data))
                        {
                            var actual = example.Learn<VowpalWabbitScalarPrediction>();

                            Assert.AreEqual(expected.Value, actual.Value, 1e-6, "Learn output differs on line: " + lineNr);
                        }

                        lineNr++;
                    }));

                vwStr.RunMultiPass();
                vw.RunMultiPass();

                vwStr.SaveModel("models/str0001.model");
                vw.SaveModel("models/0001.model");
                 
                VWTestHelper.AssertEqual(@"train-sets\ref\0001.stderr", vwStr.PerformanceStatistics);
                VWTestHelper.AssertEqual(@"train-sets\ref\0001.stderr", vw.PerformanceStatistics);
            }

            Assert.AreEqual(input.Count, references.Length);

            using (var vwStr = new VowpalWabbit("-k -t -i models/str0001.model --invariant"))
            using (var vw = new VowpalWabbit<Test1>("-k -t -i models/0001.model --invariant"))
            {
                for (var i = 0; i < input.Count; i++)
                {
                    var expected = vwStr.Predict<VowpalWabbitScalarPrediction>(input[i].Line);

                    using (var example = vw.ReadExample(input[i]))
                    {
                        var actual = example.Predict<VowpalWabbitScalarPrediction>();

                        Assert.AreEqual(expected.Value, actual.Value, 1e-5);

                        Assert.AreEqual(
                            references[i],
                            actual.Value,
                            1e-5,
                            string.Format(CultureInfo.InvariantCulture, "Expected {0} vs. actual {1} at line {2}", references[i], actual.Value, i));
                    }
                }

                VWTestHelper.AssertEqual(@"test-sets\ref\0001.stderr", vwStr.PerformanceStatistics);
                VWTestHelper.AssertEqual(@"test-sets\ref\0001.stderr", vw.PerformanceStatistics);
            }
        }

        //[TestMethod]
        //[Ignore]
        //[DeploymentItem(@"train-sets\rcv1_cb_eval", "train-sets")]
        //public void Test74()
        //{
        //    // 2 1:1:0.5 | tuesday year million short compan vehicl line stat financ commit exchang plan corp subsid credit issu debt pay gold bureau prelimin refin billion telephon time draw basic relat file spokesm reut secur acquir form prospect period interview regist toront resourc barrick ontario qualif bln prospectus convertibl vinc borg arequip
        //    using (var vw = new VowpalWabbit<Rcv1CbEval>("--cb 2 --eval"))
        //    using (var fr = new StreamReader(@"train-sets\rcv1_cb_eval"))
        //    {
        //        string line;

        //        while ((line = fr.ReadLine()) != null)
        //        {
        //            var parts = line.Split('|');

        //            var data = new Rcv1CbEval()
        //            {
        //                Words = parts[1].Split(' ')
        //            }; 

        //            using(var example = vw.ReadExample(data))
        //            {
        //                example.AddLabel(parts[0]);
        //                example.Learn();
        //            }
        //        }
        //    }
        //}
        

    }


    public class Test1 : IExample
    {
        [Feature(FeatureGroup = 'f', Namespace = "eatures", Name = "const", Order = 2)]
        public float Constant { get; set; }

        [Feature(FeatureGroup = 'f', Namespace = "eatures", Order = 1)]
        public IList<KeyValuePair<string, float>> Features { get; set; }

        public string Line { get; set; }

        public ILabel Label { get; set;}
    }

    public class Rcv1CbEval
    {
        [Feature]
        public string[] Words { get; set; } 
    }

    public class MyListener : VowpalWabbitBaseListener
    {
        private Test1 example;

        private Action<Test1> action;

        public MyListener(Action<Test1> action)
        {
            this.action = action;
        }

        public override void EnterExample(VowpalWabbitParser.ExampleContext context)
        {
            this.example = new Test1()
            {
                Features = new List<KeyValuePair<string, float>>()
            };
        }

        public override void ExitExample(VowpalWabbitParser.ExampleContext context)
        {
            this.example.Line = context.GetText();
            this.action(this.example);
        }

        public override void ExitLabel_simple(VowpalWabbitParser.Label_simpleContext context)
        {
            this.example.Label = new SimpleLabel()
            {
                Label = float.Parse(context.value.Text, CultureInfo.InvariantCulture)
            };
        }

        public override void ExitFeature(VowpalWabbitParser.FeatureContext context)
        {
            var index = context.index.Text;
            var x = float.Parse(context.x.Text, CultureInfo.InvariantCulture);

            if (index == "const")
            {
                this.example.Constant = x;
            }
            else
            {
                this.example.Features.Add(new KeyValuePair<string, float>(index, x));
            }
        }
    }
}

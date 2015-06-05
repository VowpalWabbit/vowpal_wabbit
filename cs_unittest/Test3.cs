using Antlr4.Runtime;
using Antlr4.Runtime.Tree;
using cs_test;
using Microsoft.Research.MachineLearning;
using Microsoft.Research.MachineLearning.Interfaces;
using Microsoft.Research.MachineLearning.Labels;
using Microsoft.Research.MachineLearning.Serializer.Attributes;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace cs_unittest
{
    [TestClass]
    public class Test3Class : TestBase
    {
        [TestMethod]
        [DeploymentItem(@"train-sets\0002.dat", "train-sets")]
        [DeploymentItem(@"train-sets\ref\0002.stderr", @"train-sets\ref")]
        public void Test3()
        {
            using (var vw = new VowpalWabbit<Data>("-k train-sets/0002.dat -f models/0002.model --invariant"))
            {
                VWTestHelper.ParseInput(
                    File.OpenRead(@"train-sets\0002.dat"),
                    new MyListener3(x => 
                        {
                            using (var ex = vw.ReadExample(x))
                            {
                                ex.Learn<VowpalWabbitPredictionNone>();
                            }
                        }));
            
                VWTestHelper.AssertEqual(@"train-sets\ref\0002.stderr", vw.PerformanceStatistics);
            }
        }

        [TestMethod]
        public void TestAntlr()
        {
            var line1 = "0.521144 1 PFF/20091028|T PFF |f t1:-0.0236849 t5:-0.10215 r5:0.727735 t10:-0.0387662 r10:0.911208 t20:-0.00777943 r20:0.952668 t40:0.014542 r40:0.832479 t60:0.00395449 r60:0.724504 t90:0.0281418 r90:0.784653";
            VWTestHelper.ParseInput(new MemoryStream(Encoding.UTF8.GetBytes(line1)),
                new MyListener3(x => {
                    Assert.AreEqual("PFF", x.T);
                    var label = x.Label as SimpleLabel;
                    Assert.AreEqual(0.521144, label.Label, 1e-5);
                    Assert.AreEqual(1, label.Initial);

                    Assert.AreEqual(13, x.F.Count);
                    Assert.AreEqual("t1", x.F[0].Key);
                    Assert.AreEqual(-0.0236849, x.F[0].Value, 1e-5);
                }));
        }

        public class Data : IExample
        {
            [Feature(FeatureGroup = 'T', Name = "")]
            public string T { get; set; }

            [Feature(FeatureGroup = 'f')]
            public List<KeyValuePair<string, float>> F { get; set; }

            public ILabel Label
            {
                get;
                set;
            }
        }

        public class MyListener3 : VowpalWabbitBaseListener
        {
            private Data example;

            private Action<Data> action;

            public MyListener3(Action<Data> action)
            {
                this.action = action;
            }

            public override void EnterExample(VowpalWabbitParser.ExampleContext context)
            {
                this.example = new Data()
                {
                    F = new List<KeyValuePair<string, float>>()
                };
            }

            public override void ExitExample(VowpalWabbitParser.ExampleContext context)
            {
                this.action(this.example);
            }

            public override void ExitLabel_simple(VowpalWabbitParser.Label_simpleContext context)
            {
                this.example.Label = new SimpleLabel()
                {
                    Label = float.Parse(context.value.Text, CultureInfo.InvariantCulture),
                    Initial = float.Parse(context.initial.Text, CultureInfo.InvariantCulture)
                };
            }

            public override void ExitFeature(VowpalWabbitParser.FeatureContext context)
            {
                var index = context.index;

                if (index == null)
                {
                    // dense feature
                }
                else
                {
                    var weight_index = index.Text;
                    var x = context.x;
                    if (x == null)
                    {
                        // hashed feature
                        this.example.T = weight_index;
                    }
                    else
                    {
                        // sparse feature
                        var xval = float.Parse(x.Text, CultureInfo.InvariantCulture);
                        this.example.F.Add(new KeyValuePair<string, float>(weight_index, xval));
                    }
                }
            }
        }
    }
}

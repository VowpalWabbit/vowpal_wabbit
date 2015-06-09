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

            var line2 = "1 |f 13:3.9656971e-02 24:3.4781646e-02 69:4.6296168e-02 85:6.1853945e-02 140:3.2349996e-02 156:1.0290844e-01 175:6.8493910e-02 188:2.8366476e-02 229:7.4871540e-02 230:9.1505975e-02 234:5.4200061e-02 236:4.4855952e-02 238:5.3422898e-02 387:1.4059304e-01 394:7.5131744e-02 433:1.1118756e-01 434:1.2540409e-01 438:6.5452829e-02 465:2.2644201e-01 468:8.5926279e-02 518:1.0214076e-01 534:9.4191484e-02 613:7.0990764e-02 646:8.7701865e-02 660:7.2289191e-02 709:9.0660661e-02 752:1.0580081e-01 757:6.7965068e-02 812:2.2685185e-01 932:6.8250686e-02 1028:4.8203137e-02 1122:1.2381379e-01 1160:1.3038123e-01 1189:7.1542501e-02 1530:9.2655659e-02 1664:6.5160148e-02 1865:8.5823394e-02 2524:1.6407280e-01 2525:1.1528353e-01 2526:9.7131468e-02 2536:5.7415009e-01 2543:1.4978983e-01 2848:1.0446861e-01 3370:9.2423186e-02 3960:1.5554591e-01 7052:1.2632671e-01 16893:1.9762035e-01 24036:3.2674628e-01 24303:2.2660980e-010";
            VWTestHelper.ParseInput(new MemoryStream(Encoding.UTF8.GetBytes(line2)),
                new MyListener3(x =>
                {
                    var label = x.Label as SimpleLabel;
                    Assert.AreEqual(1, label.Label, 1e-5);

                    Assert.AreEqual(49, x.F.Count);

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
                var simpleLabel = new SimpleLabel()
                {
                    Label = context.value.value
                };

                if (context.initial != null)
                {
                    simpleLabel.Initial = context.initial.value;
                }

                this.example.Label = simpleLabel;
            }

            public override void ExitNumber(VowpalWabbitParser.NumberContext context)
            {
                context.value = float.Parse(context.GetText(), CultureInfo.InvariantCulture);
            }

            public override void ExitFeatureSparse(VowpalWabbitParser.FeatureSparseContext context)
            {
                var index = context.index;

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
                    this.example.F.Add(new KeyValuePair<string, float>(weight_index, context.x.value));
                }
            }
        }
    }
}

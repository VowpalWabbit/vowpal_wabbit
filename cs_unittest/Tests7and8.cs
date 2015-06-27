using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using VW.Interfaces;
using VW.Labels;
using VW.Serializer.Attributes;

namespace cs_unittest
{
    [TestClass]
    public class Tests7and8 : TestBase
    {
        [TestMethod]
        [Description("using normalized adaptive updates and a low --power_t")]
        [DeploymentItem(@"train-sets\0002.dat", "train-sets")]
        [DeploymentItem(@"train-sets\ref\0002c.stderr", @"train-sets\ref")]
        [DeploymentItem(@"pred-sets\ref\0002c.predict", @"pred-sets\ref")]
        public void Test7and8()
        {
            VWTestHelper.Learn<Data2, DataListener2>(
                "-k --power_t 0.45 -f models/0002c.model",
                @"train-sets\0002.dat",
                @"train-sets\ref\0002c.stderr");

            VWTestHelper.Predict<Data2, DataListener2>(
                "-k -t -i models/0002c.model",
                @"train-sets\0002.dat",
                @"pred-sets\ref\0002c.predict");
        }
    }

    public class Data2 : BaseData, IExample
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

    public class DataListener2 : VowpalWabbitListenerToEvents<Data2>
    {
        private Data2 example;

        public override void EnterExample(VowpalWabbitParser.ExampleContext context)
        {
            this.example = new Data2()
            {
                F = new List<KeyValuePair<string, float>>()
            };
        }

        public override void ExitExample(VowpalWabbitParser.ExampleContext context)
        {
            this.example.Line = context.GetText();
            this.Created(this.example);
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

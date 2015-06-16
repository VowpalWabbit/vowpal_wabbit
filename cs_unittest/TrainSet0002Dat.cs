using cs_unittest;
using Microsoft.Research.MachineLearning.Interfaces;
using Microsoft.Research.MachineLearning.Labels;
using Microsoft.Research.MachineLearning.Serializer.Attributes;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TrainSet0002Dat
{
    public class Data : BaseData, IExample
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

    public class DataListener : VowpalWabbitListenerToEvents<Data>
    {
        private Data example;

        public override void EnterExample(VowpalWabbitParser.ExampleContext context)
        {
            this.example = new Data()
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

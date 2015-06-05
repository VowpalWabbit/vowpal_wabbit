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
using System.IO.Compression;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace cs_unittest
{
    [TestClass]
    public class Rcv1TestClass
    {
        [TestMethod]
        [Ignore]
        public void Rcv1Test()
        {
            var stopwatch = new Stopwatch();
            stopwatch.Start();
            using (var gz = new GZipStream(File.OpenRead(@"D:\Data\rcv1.train.vw.gz"), CompressionMode.Decompress))
            {
                using (var vw = new VowpalWabbit<Data>("-k -f rcv1.model -c rcv1.cache"))
                {
                    VWTestHelper.ParseInput(gz, new MyListener(vw));
                }
                // 1 |f 13:3.9656971e-02 24:3.4781646e-02 69:4.6296168e-02 85:6.1853945e-02 140:3.2349996e-02 156:1.0290844e-01 
            
            }

            Console.WriteLine(stopwatch.Elapsed);
        }

        public class MyListener : VowpalWabbitBaseListener
        {
            private Data example;

            private VowpalWabbit<Data> vw;

            public MyListener(VowpalWabbit<Data> vw)
            {
                this.vw = vw;

                // re-use example object
                this.example = new Data()
                {
                    Features = new List<KeyValuePair<string, float>>()
                };
            }

            public override void EnterExample(VowpalWabbitParser.ExampleContext context)
            {
                this.example.Features.Clear();
            }

            public override void ExitFeature(VowpalWabbitParser.FeatureContext context)
            {
                this.example.Features.Add(
                    new KeyValuePair<string, float>(
                        context.index.Text, 
                        float.Parse(context.x.Text, CultureInfo.InvariantCulture)));
            }

            public override void ExitLabel_simple(VowpalWabbitParser.Label_simpleContext context)
            {
                this.example.Label = new SimpleLabel
                {
                    Label = float.Parse(context.value.Text, CultureInfo.InvariantCulture)
                };
            }

            public override void ExitExample(VowpalWabbitParser.ExampleContext context)
            {
                using (var vwExample = this.vw.ReadExample(this.example))
                {
                    vwExample.Learn<VowpalWabbitPredictionNone>();
                }
            }
        }

        public class Data : IExample
        {
            [Feature(FeatureGroup = 'f')]
            public IList<KeyValuePair<string, float>> Features { get; set; }

            public ILabel Label { get; set; }
        }
    }
}

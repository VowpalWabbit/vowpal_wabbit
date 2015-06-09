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
        public void Rcv1Test()
        {
            using (var gz = new GZipStream(File.OpenRead(@"D:\Data\rcv1.train.vw.gz"), CompressionMode.Decompress))
            {
                MyListener listener;
                using (var vw = new VowpalWabbit<Data>("-k -f rcv1.model -c rcv1.cache"))
                {
                    listener = new MyListener(vw);
                    VWTestHelper.ParseInput(gz, listener);
                }
                // 1 |f 13:3.9656971e-02 24:3.4781646e-02 69:4.6296168e-02 85:6.1853945e-02 140:3.2349996e-02 156:1.0290844e-01 

                Console.WriteLine(listener.stopwatch.Elapsed);
            }
        }

        public class MyListener : VowpalWabbitBaseListener
        {
            internal Stopwatch stopwatch;

            private Data example;

            private VowpalWabbit<Data> vw;

            private int lines;

            public MyListener(VowpalWabbit<Data> vw)
            {
                this.vw = vw;

                // re-use example object
                this.example = new Data()
                {
                    Features = new List<KeyValuePair<string, float>>()
                };

                this.stopwatch = new Stopwatch();
            }

            public override void EnterExample(VowpalWabbitParser.ExampleContext context)
            {
                if (this.lines++ > 1000)
                {
                    Environment.Exit(0);
                }

                this.example.Features.Clear();
            }

            public override void ExitNumber(VowpalWabbitParser.NumberContext context)
            {
                context.value = float.Parse(context.GetText(), CultureInfo.InvariantCulture);
            }

            public override void ExitFeatureSparse(VowpalWabbitParser.FeatureSparseContext context)
            {
                this.example.Features.Add(
                    new KeyValuePair<string, float>(
                        context.index.Text, 
                        context.x.value));
            }

            public override void ExitLabel_simple(VowpalWabbitParser.Label_simpleContext context)
            {
                this.example.Label = new SimpleLabel
                {
                    Label = context.value.value
                };
            }

            public override void ExitExample(VowpalWabbitParser.ExampleContext context)
            {
                this.stopwatch.Start();
                using (var vwExample = this.vw.ReadExample(this.example))
                {
                    vwExample.Learn<VowpalWabbitPredictionNone>();
                }
                this.stopwatch.Stop();
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

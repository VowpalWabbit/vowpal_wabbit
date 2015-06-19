using VW;
using VW.Interfaces;
using VW.Labels;
using VW.Serializer.Attributes;
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
            using (var gz = new GZipStream(File.OpenRead(@"D:\Data\rcv1.train.vw.gz"), CompressionMode.Decompress))
            using (var reader = new StreamReader(gz))
            {
                MyListener listener;
                using (var vw = new VowpalWabbit<Data>("-k -f rcv1.model -c rcv1.cache"))
                {
                    listener = new MyListener(vw);
                    string line;

                    while( (line = reader.ReadLine()) != null)
                    {
                        VWTestHelper.ParseInput(line, listener);
                    }
                }
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
                    vwExample.Learn();
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

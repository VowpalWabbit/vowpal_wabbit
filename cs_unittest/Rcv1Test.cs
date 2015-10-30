using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.IO.Compression;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using VW;
using VW.Interfaces;
using VW.Labels;
using VW.Serializer.Attributes;

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

                    while ((line = reader.ReadLine()) != null)
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

            private SimpleLabel label;

            private VowpalWabbit<Data> vw;

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
                this.label = null;
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
                this.label = new SimpleLabel
                {
                    Label = context.value.value
                };
            }

            public override void ExitExample(VowpalWabbitParser.ExampleContext context)
            {
                this.stopwatch.Start();
                this.vw.Learn(this.example, this.label);
                this.stopwatch.Stop();
            }
        }

        public class Data
        {
            [Feature(FeatureGroup = 'f')]
            public IList<KeyValuePair<string, float>> Features { get; set; }
        }
    }
}

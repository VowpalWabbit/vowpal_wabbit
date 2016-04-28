using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW;
using VW.Interfaces;
using VW.Serializer;

namespace cs_unittest
{
    internal sealed class VowpalWabbitExampleJsonValidator : IDisposable
    {
        private VowpalWabbit vw;

        internal VowpalWabbitExampleJsonValidator(string args = null) : this(new VowpalWabbitSettings(args))
        {
        }

        internal VowpalWabbitExampleJsonValidator(VowpalWabbitSettings settings)
        {
            this.vw = new VowpalWabbit(settings.ShallowCopy(enableStringExampleGeneration: true));
        }

        public void Validate(string line, string json, IVowpalWabbitLabelComparator labelComparator = null, ILabel label = null)
        {
            using (var strExample = this.vw.ParseLine(line))
            using (var jsonSerializer = new VowpalWabbitJsonSerializer(this.vw))
            using (var jsonExample = (VowpalWabbitSingleLineExampleCollection)jsonSerializer.ParseAndCreate(json, label))
            using (var strJsonExample = this.vw.ParseLine(jsonExample.Example.VowpalWabbitString))
            {
                var diff = strExample.Diff(this.vw, jsonExample.Example, labelComparator);
                Assert.IsNull(diff, diff + " generated string: '" + jsonExample.VowpalWabbitString + "'");

                diff = strExample.Diff(this.vw, strJsonExample, labelComparator);
                Assert.IsNull(diff, diff);
            }
        }

        public void Validate(string[] lines, string json, IVowpalWabbitLabelComparator labelComparator = null, ILabel label = null, int? index = null)
        {
            VowpalWabbitExample[] strExamples = new VowpalWabbitExample[lines.Count()];

            try
            {
                for (int i = 0; i < lines.Length; i++)
                    strExamples[i] = this.vw.ParseLine(lines[i]);

                using (var jsonSerializer = new VowpalWabbitJsonSerializer(this.vw))
                using (var jsonExample = (VowpalWabbitMultiLineExampleCollection)jsonSerializer.ParseAndCreate(json, label, index))
                {
                    var jsonExamples = new List<VowpalWabbitExample>();

                    if (jsonExample.SharedExample != null)
                        jsonExamples.Add(jsonExample.SharedExample);

                    jsonExamples.AddRange(jsonExample.Examples);

                    Assert.AreEqual(strExamples.Length, jsonExamples.Count);

                    for (int i = 0; i < strExamples.Length; i++)
                    {
                        using (var strJsonExample = this.vw.ParseLine(jsonExamples[i].VowpalWabbitString))
                        {
                            var diff = strExamples[i].Diff(this.vw, jsonExamples[i], labelComparator);
                            Assert.IsNull(diff, diff + " generated string: '" + jsonExamples[i].VowpalWabbitString + "'");

                            diff = strExamples[i].Diff(this.vw, strJsonExample, labelComparator);
                            Assert.IsNull(diff, diff);
                        }
                    }
                }
            }
            finally
            {
                foreach (var ex in strExamples)
                    if (ex != null)
                        ex.Dispose();
            }
        }

        public void Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        private void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (this.vw != null)
                {
                    this.vw.Dispose();
                    this.vw = null;
                }
            }
        }
    }
}

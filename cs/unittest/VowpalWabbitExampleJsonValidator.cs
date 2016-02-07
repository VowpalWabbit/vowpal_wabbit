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
            {
                jsonSerializer.Parse(json, label);
                using (var jsonExample = jsonSerializer.CreateExample())
                using (var strJsonExample = this.vw.ParseLine(jsonExample.VowpalWabbitString))
                {
                    var diff = strExample.Diff(this.vw, jsonExample, labelComparator);
                    Assert.IsNull(diff, diff + " generated string: '" + jsonExample.VowpalWabbitString + "'");

                    diff = strExample.Diff(this.vw, strJsonExample, labelComparator);
                    Assert.IsNull(diff, diff);
                }
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

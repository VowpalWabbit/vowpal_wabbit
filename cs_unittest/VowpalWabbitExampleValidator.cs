using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using VW;
using VW.Interfaces;
using VW.Labels;
using VW.Serializer;

namespace cs_unittest
{
    internal sealed class VowpalWabbitExampleValidator<TExample> : IDisposable
    {
        private VowpalWabbit<TExample> vw;
        private Action<VowpalWabbitMarshalContext, TExample, ILabel> serializer;

        internal VowpalWabbitExampleValidator(string args)
        {
            // remove model writing
            args = Regex.Replace(args, @"-f\s+[^ -]+", " ");

            // remove cache file
            args = Regex.Replace(args, @"-c\s+([^ -]+)?", " ");

            this.vw = new VowpalWabbit<TExample>(new VowpalWabbitSettings(args, enableStringExampleGeneration: true));
            this.serializer = vw.Serializer.Func(this.vw.Native);
        }

        public void Validate(string line, TExample example, ILabel label = null)
        {
            IVowpalWabbitLabelComparator comparator;

            if (label == null)
            {
                comparator = null;
            }
            else if (label is SimpleLabel || label == SharedLabel.Instance)
            {
                comparator = VowpalWabbitLabelComparator.Simple;
            }
            else if (label is ContextualBanditLabel)
            {
                comparator = VowpalWabbitLabelComparator.ContextualBandit;
            }
            else
            {
                throw new ArgumentException("Label type not supported: " + label.GetType());
            }

            using (var context = new VowpalWabbitMarshalContext(this.vw.Native))
            {
                // validate string serializer
                this.serializer(context, example, label);

                // natively parsed string example compared against:
                // (1) natively build example
                // (2) string serialized & natively parsed string example
                using (var strExample = this.vw.Native.ParseLine(line))
                using (var strConvertedExample = this.vw.Native.ParseLine(context.StringExample.ToString()))
                using (var nativeExample = context.ExampleBuilder.CreateExample())
                {
                    var diff = strExample.Diff(this.vw.Native, strConvertedExample, comparator);
                    Assert.IsNull(diff, diff + " generated string: '" + context.StringExample + "'");

                    diff = strExample.Diff(this.vw.Native, nativeExample, comparator);
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

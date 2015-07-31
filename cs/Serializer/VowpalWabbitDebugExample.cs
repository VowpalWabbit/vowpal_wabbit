using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW;
using VW.Serializer.Visitors;

namespace VW.Serializer
{
    [DebuggerDisplay("CmdLine: {VowpalWabbitString}")]
    public class VowpalWabbitDebugExample : IVowpalWabbitExample
    {
        private IVowpalWabbitExample example;

        internal VowpalWabbitDebugExample(IVowpalWabbitExample example, string vwString)
        {
            this.example = example;
            this.VowpalWabbitString = vwString;
        }

        public VowpalWabbitExample UnderlyingExample
        {
            get
            {
                return this.example.UnderlyingExample;
            }
        }

        void IDisposable.Dispose()
        {
            // return example to cache.
            this.example.Dispose();
        }

        void IVowpalWabbitExample.Learn()
        {
            this.example.Learn();
        }

        TPrediction IVowpalWabbitExample.LearnAndPredict<TPrediction>()
        {
            return this.example.LearnAndPredict<TPrediction>();
        }

        void IVowpalWabbitExample.PredictAndDiscard()
        {
            this.example.PredictAndDiscard();
        }

        TPrediction IVowpalWabbitExample.Predict<TPrediction>()
        {
            return this.example.Predict<TPrediction>();
        }

        [EditorBrowsable(EditorBrowsableState.Never)]
        public string VowpalWabbitString 
        {
            get;
            private set;
        }
    }
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Microsoft.Research.MachineLearning
{
    public class VowpalWabbitExample : IVowpalWabbitExample
    {
        protected readonly VowpalWabbit vw;

        internal VowpalWabbitExample(VowpalWabbit vw, IntPtr example) : this(vw)
        {
            this.Ptr = example;
        }

        protected VowpalWabbitExample(VowpalWabbit vw)
        {
            this.vw = vw;
        }
 
        public IntPtr Ptr { get; protected set; }

        public void AddLabel(string label)
        {
            VowpalWabbitNative.AddLabel(this.vw.vw, this.Ptr, label);
        }

        public void AddLabel(float label = float.MaxValue, float weight = 1, float initial = 0)
        {
            VowpalWabbitNative.AddLabel(this.Ptr, label, weight, initial);
        }

        public void Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            // Free managed resources
            if (this.vw != null && this.Ptr != IntPtr.Zero)
            {
                VowpalWabbitNative.FinishExample(this.vw.vw, this.Ptr);
                this.Ptr = IntPtr.Zero;
            }
        }
    }
}

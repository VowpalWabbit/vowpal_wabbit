using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Microsoft.Research.MachineLearning
{
    public class VowpalWabbitExample : IVowpalWabbitExample
    {
        private VowpalWabbit vw;

        internal VowpalWabbitExample(VowpalWabbit vw, IntPtr example)
        {
            this.vw = vw;
            this.Ptr = example;
        }
 
        public IntPtr Ptr { get; private set; }

        public void Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        private void Dispose(bool disposing)
        {
            if (this.Ptr != null)
            {
                VowpalWabbitNative.FinishExample(this.vw.vw, this.Ptr);
                this.Ptr = IntPtr.Zero;
            }
        }
    }
}

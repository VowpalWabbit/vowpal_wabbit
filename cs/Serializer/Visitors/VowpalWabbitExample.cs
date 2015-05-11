using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Microsoft.Research.MachineLearning.Serializer.Visitors
{
    public class VowpalWabbitExample : IDisposable
    {
        private VowpalWabbitNative.FEATURE_SPACE[] featureSpace;

        internal VowpalWabbitExample(VowpalWabbitNative.FEATURE_SPACE[] featureSpace)
        {
            this.featureSpace = featureSpace;
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
                // Free managed resources
            }

            // Free unmanaged resources
        }
    }
}

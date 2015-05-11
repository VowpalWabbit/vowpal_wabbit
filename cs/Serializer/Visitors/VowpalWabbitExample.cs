using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace Microsoft.Research.MachineLearning.Serializer.Visitors
{
    public class VowpalWabbitExample : IDisposable
    {
        private GCHandle[] handles;

        internal VowpalWabbitExample(VowpalWabbitNative.FEATURE_SPACE[] featureSpace, GCHandle[] handles)
        {
            this.FeatureSpace = featureSpace;
            this.handles = handles;
        }

        public VowpalWabbitNative.FEATURE_SPACE[] FeatureSpace { get; private set; }

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

                if (this.handles != null)
                {
                    foreach (var handle in this.handles)
                    {
                        handle.Free();
                    }
                }
            }

            // Free unmanaged resources
        }
    }
}

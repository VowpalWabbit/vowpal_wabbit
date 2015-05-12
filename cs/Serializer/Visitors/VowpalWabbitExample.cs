using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace Microsoft.Research.MachineLearning.Serializer.Visitors
{
    /// <summary>
    /// Manages features spaces.
    /// </summary>
    public class VowpalWabbitExample : IDisposable
    {
        /// <summary>
        /// GCHandles to safely pass memory to VW. 
        /// </summary>
        private GCHandle[] handles;
        private GCHandle featureSpaceHandle;

        internal VowpalWabbitExample(VowpalWabbitNative.FEATURE_SPACE[] featureSpace, GCHandle[] handles)
        {
            this.FeatureSpace = featureSpace;
            this.handles = handles;
            this.featureSpaceHandle = GCHandle.Alloc(FeatureSpace, GCHandleType.Pinned);
        }

        /// <summary>
        /// 
        /// </summary>
        public VowpalWabbitNative.FEATURE_SPACE[] FeatureSpace { get; private set; }

        public IntPtr FeatureSpacePtr 
        { 
            get 
            { 
                return this.featureSpaceHandle.AddrOfPinnedObject();  
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
                // Free managed resources

                if (this.handles != null)
                {
                    foreach (var handle in this.handles)
                    {
                        handle.Free();
                    }
                }

                this.featureSpaceHandle.Free();
            }
        }
    }
}

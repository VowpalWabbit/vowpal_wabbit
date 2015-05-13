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
    public class VowpalWabbitNativeExample : VowpalWabbitExample
    {
        /// <summary>
        /// GCHandles to safely pass memory to VW. 
        /// </summary>
        private GCHandle[] handles;
        private GCHandle featureSpaceHandle;
        private VowpalWabbitNative.FEATURE_SPACE[] featureSpace;

        internal VowpalWabbitNativeExample(VowpalWabbit vw, VowpalWabbitNative.FEATURE_SPACE[] featureSpace, GCHandle[] handles) 
            : base(vw)
        {
            this.featureSpace = featureSpace;
            this.handles = handles;
            this.featureSpaceHandle = GCHandle.Alloc(featureSpace, GCHandleType.Pinned);

            this.Ptr = VowpalWabbitNative.ImportExample(
                this.vw.vw,
                this.featureSpaceHandle.AddrOfPinnedObject(),
                (IntPtr)this.featureSpace.Length);
        }

        internal bool IsEmpty
        {
            get { return featureSpace.Length == 0; }
        }
        
        protected override void Dispose(bool disposing)
        {
            // Free unmanaged resources
            if (this.handles != null)
            {
                foreach (var handle in this.handles)
                {
                    handle.Free();
                }
                this.handles = null;
            }

            this.featureSpaceHandle.Free();
        }
    }
}

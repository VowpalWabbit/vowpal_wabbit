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
    internal class VowpalWabbitNativeExample : IVowpalWabbitExample
    {
        /// <summary>
        /// GCHandles to safely pass memory to VW. 
        /// </summary>
        private GCHandle[] handles;
        private GCHandle featureSpaceHandle;
        private VowpalWabbitNative.FEATURE_SPACE[] featureSpace;
        private VowpalWabbit vw;

        internal VowpalWabbitNativeExample(VowpalWabbitNative.FEATURE_SPACE[] featureSpace, GCHandle[] handles)
        {
            this.featureSpace = featureSpace;
            this.handles = handles;
            this.featureSpaceHandle = GCHandle.Alloc(featureSpace, GCHandleType.Pinned);
        }

        internal void ImportInto(VowpalWabbit vw)
        {
            if (this.vw != null && this.Ptr != IntPtr.Zero)
            {
                VowpalWabbitNative.FinishExample(this.vw.vw, this.Ptr);
            }

            this.vw = vw;

            this.Ptr = VowpalWabbitNative.ImportExample(
                this.vw.vw, 
                this.featureSpaceHandle.AddrOfPinnedObject(), 
                (IntPtr)this.featureSpace.Length);
        }

        public IntPtr Ptr
        {
            get;
            private set;
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
                    this.handles = null;
                }

                this.featureSpaceHandle.Free();

                if (this.vw != null && this.Ptr != IntPtr.Zero)
                {
                    VowpalWabbitNative.FinishExample(this.vw.vw, this.Ptr);
                    this.Ptr = IntPtr.Zero;
                }
            }
        }

    }
}

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace Microsoft.Research.MachineLearning
{
    public class VowpalWabbitModel : IDisposable
    {
        private IntPtr vw;

        public VowpalWabbitModel(Stream model)
        {
            // TODO: initialize with byte array
            // this.vw = VowpalWabbitNative.Initialize(arguments);
            // TODO: export model
            // this.ModelPtr = VowpalWabbitNative.ExportModel(vw);
        }

        internal IntPtr ModelPtr
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
            }

            // Free unmanaged resources
            if (this.vw != IntPtr.Zero)
            {
                VowpalWabbitNative.Finish(this.vw);
                this.vw = IntPtr.Zero;
            }
        }
    }
}

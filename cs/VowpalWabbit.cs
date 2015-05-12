using Microsoft.Research.MachineLearning.Serializer;
using Microsoft.Research.MachineLearning.Serializer.Visitor;
using Microsoft.Research.MachineLearning.Serializer.Visitors;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace Microsoft.Research.MachineLearning
{
    public class VowpalWabbit : IDisposable
    {
        protected IntPtr vw;

        public VowpalWabbit(string arguments)
        {
            this.vw = VowpalWabbitNative.Initialize(arguments);
        }

        public VowpalWabbit(VowpalWabbitModel model)
        {
            // TODO: initialize VW using shared model
        }

        public float GetCostSensitivePrediction(IntPtr example)
        {
            VowpalWabbitNative.Predict(vw, example);
            VowpalWabbitNative.FinishExample(vw, example);

            return VowpalWabbitNative.GetCostSensitivePrediction(example);
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
    
    public sealed class VowpalWabbit<TExample> : VowpalWabbit
    {
        private readonly Func<TExample, VowpalWabbitNativeVisitor, IList<VowpalWabbitExample>> serializer;
        private readonly VowpalWabbitNativeVisitor visitor;

        public VowpalWabbit(string arguments) : base(arguments)
        {
            this.visitor = new VowpalWabbitNativeVisitor();

            // Compile serializer
            this.serializer = VowpalWabbitSerializer.CreateSerializer<TExample, VowpalWabbitNativeVisitor, VowpalWabbitExample, VowpalWabbitNative.FEATURE[], IEnumerable<VowpalWabbitNative.FEATURE>>();
        }
        
        public float GetCostSensitivePrediction(TExample example)
        {
            // TODO: support action dependent features
            using (var vwExample = this.serializer(example, this.visitor)[0])
            {
                var importedExample = VowpalWabbitNative.ImportExample(this.vw, vwExample.FeatureSpacePtr, (IntPtr)vwExample.FeatureSpace.Length);

                return base.GetCostSensitivePrediction(importedExample);
            }
        }
    }

    public sealed class VowpalWabbitString<TExample> : VowpalWabbit
    {
        private readonly Func<TExample, VowpalWabbitStringVisitor, IList<string>> serializer;
        private readonly VowpalWabbitStringVisitor stringVisitor;

        public VowpalWabbitString(string arguments)
            : base(arguments)
        {
            this.stringVisitor = new VowpalWabbitStringVisitor();
            // Compile serializer
            this.serializer = VowpalWabbitSerializer.CreateSerializer<TExample, VowpalWabbitStringVisitor, string, string, string>();
        }
        
        public float GetCostSensitivePrediction(TExample example)
        {
            // TODO: support action dependent features
            var exampleLine = this.serializer(example, this.stringVisitor)[0];

            var vwExample = VowpalWabbitNative.ReadExample(this.vw, exampleLine);

            return base.GetCostSensitivePrediction(vwExample);
        }
    }
}

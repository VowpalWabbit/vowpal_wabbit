using Microsoft.Research.MachineLearning.Serializer;
using Microsoft.Research.MachineLearning.Serializer.Visitor;
using System;
using System.Collections.Generic;
using System.Linq;
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

        public VowpalWabbitModel(VowpalWabbitModel model)
        {

        }

        public float GetCostSensitivePrediction(string exampleLine)
        {
            IntPtr example = VowpalWabbitNative.ReadExample(vw, exampleLine);

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
        private readonly Action<TExample, VowpalWabbitStringVisitor> serializer;

        public VowpalWabbit(string arguments) : base(arguments)
        {
            // Compile serializer
            this.serializer = VowpalWabbitSerializer.CreateSerializer<TExample, VowpalWabbitStringVisitor>();
        }

        private string GetExampleLine(TExample example)
        {
            var visitor = new VowpalWabbitStringVisitor();
            this.serializer(example, visitor);
            return visitor.ExampleLine;
        }

        public float GetCostSensitivePrediction(TExample example)
        {
            var exampleLine = GetExampleLine(example);
            return base.GetCostSensitivePrediction(exampleLine);
        }
    }
}

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
        internal IntPtr vw;

        public VowpalWabbit(string arguments)
        {
            this.vw = VowpalWabbitNative.Initialize(arguments);
        }

        public VowpalWabbit(VowpalWabbitModel model)
        {
            // TODO: initialize VW using shared model
        }

        public IVowpalWabbitExample ReadExample(string line)
        {
            var ptr = VowpalWabbitNative.ReadExample(this.vw, line);
            return new VowpalWabbitExample(this, ptr);
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
            //if (disposing)
            //{
            //    // Free managed resources
            //}

            // Free unmanaged resources
            if (this.vw != IntPtr.Zero)
            {
                VowpalWabbitNative.Finish(this.vw);
                this.vw = IntPtr.Zero;
            }
        }
    }
    
    public class VowpalWabbit<TExample> : VowpalWabbit
    {
        protected readonly Func<TExample, VowpalWabbitNativeVisitor, VowpalWabbitNativeExample> serializer;
        protected readonly VowpalWabbitNativeVisitor visitor;

        public VowpalWabbit(string arguments) : base(arguments)
        {
            this.visitor = new VowpalWabbitNativeVisitor();

            // Compile serializer
            this.serializer = VowpalWabbitSerializer.CreateSerializer<TExample, VowpalWabbitNativeVisitor, VowpalWabbitNativeExample, VowpalWabbitNative.FEATURE[], IEnumerable<VowpalWabbitNative.FEATURE>>();
        }
        
        public float GetCostSensitivePrediction(TExample example)
        {
            // TODO: support action dependent features
            using (var vwExample = this.serializer(example, this.visitor))
            {
                vwExample.ImportInto(this.vw);

                return base.GetCostSensitivePrediction(vwExample.Ptr);
            }
        }

        public IVowpalWabbitExample ReadExample(TExample example)
        {
            return this.serializer(example, this.visitor);
        }
    }

    public sealed class VowpalWabbit<TExample, TActionDependentFeature> : VowpalWabbit<TExample>
        where TExample : IActionDependentFeatureExample<TActionDependentFeature>
    {
        private Func<TActionDependentFeature, VowpalWabbitNativeVisitor, VowpalWabbitNativeExample> actionDependentFeatureSerializer;

        public VowpalWabbit(string arguments) : base(arguments)
        {
        }

        public void MSNTrain<T>(IActionDependentFeatureExample<T> example, int chosenActionIndex, float cost, float prob)
        {
            // TODO: where to stick the cost
            // shared|userlda: .1
            // `doc1|lda :.1 :.2
            // 0:cost:prob `doc2 |lda :.2 :.3
            // <new line>
        }

        public T[] MSNPredict<T>(IActionDependentFeatureExample<T> example)
        {
            // shared |userlda :.1 |che a:.1 
            // `doc1 |lda :.1 :.2 [1]
            // `doc2 |lda :.2 :.3 [2]
            // <new line>

            var examples = new List<VowpalWabbitNativeExample>();
            
            try
            {
                var sharedExample = this.serializer(example, this.visitor);
                examples.Add(sharedExample);

                sharedExample.ImportLabeledInto(this.vw, "shared");
                VowpalWabbitNative.Predict(this.vw, sharedExample.Ptr);

                // leave as loop so if the serializer throws an exception, anything allocated so far can be free'd
                foreach (var actionDependentFeature in example.ActionDependentFeatures)
                {
                    // TODO: insert caching here
                    var adfExample = this.actionDependentFeatureSerializer(actionDependentFeature, this.visitor);
                    actionDependentFeatures.Add(adfExample);

                    adfExample.ImportInto(this.vw);
                    VowpalWabbitNative.Predict(this.vw, adfExample.Ptr);
                }

                // allocate empty example to signal we're finished
                var emptyExample = new VowpalWabbitNativeExample(new VowpalWabbitNative.FEATURE_SPACE[0], new GCHandle[0]);
                emptyExample.ImportInto(this.vw);

                VowpalWabbitNative.Predict(this.vw, emptyExample);

                var multiLabelPrediction = new List<uint>();

                // reorder
                var result = new T[multiLabelPrediction.Count];
                for (var i = 0; i < multiLabelPrediction.Count; i++)
			    {
			        result[i] = example.ActionDependentFeatures[multiLabelPrediction[i]];
			    }

                return result;
            }
            finally
            {
                foreach (var actionDependentFeature in actionDependentFeatures)
                {
                    actionDependentFeature.Dispose();
                }
            }

            // ImportExampled(shared)
            // for all action depent
            //  ImportExample(actionDepent) if not in cache

            // ee = new EmptyExample (ImportExample)

            // Predict for shared and action dependent

            // VW.Predict(ee) // new line example
            // List<uint> VW.GetMultiLabelPrediction(ee)
            // remapping of ints into the ActionDependent Feature object - this is a subset?
            // 1-based index for action results

            return null;
        }
    }

    public sealed class VowpalWabbitString<TExample> : VowpalWabbit
    {
        private readonly Func<TExample, VowpalWabbitStringVisitor, string> serializer;
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
            var exampleLine = this.serializer(example, this.stringVisitor);

            var vwExample = VowpalWabbitNative.ReadExample(this.vw, exampleLine);

            return base.GetCostSensitivePrediction(vwExample);
        }

        public IVowpalWabbitExample ReadExample(TExample example)
        {
            var exampleLine = this.serializer(example, this.stringVisitor);

            return base.ReadExample(exampleLine);
        }
    }
}

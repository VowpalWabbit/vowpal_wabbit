using Microsoft.Research.MachineLearning.Serializer;
using Microsoft.Research.MachineLearning.Serializer.Visitors;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace Microsoft.Research.MachineLearning
{
    //public class VowpalWabbit : IDisposable
    //{
    //    internal IntPtr vw;

    //    public VowpalWabbit(string arguments)
    //    {
    //        this.vw = VowpalWabbitInterface.Initialize(arguments);
    //    }

    //    public VowpalWabbit(VowpalWabbitModel model)
    //    {
    //        // TODO: initialize VW using shared model
    //    }

    //    public IVowpalWabbitExample ReadExample(string line)
    //    {
    //        var ptr = VowpalWabbitInterface.ReadExample(this.vw, line);
    //        return new VowpalWabbitExample(this, ptr);
    //    }

    //    /// <summary>
    //    /// 
    //    /// </summary>
    //    /// <param name="example"></param>
    //    public float Learn(IVowpalWabbitExample example)
    //    {
    //        return VowpalWabbitInterface.Learn(this.vw, example.Ptr);
    //    }

    //    public float Predict(IVowpalWabbitExample example)
    //    {
    //        return VowpalWabbitInterface.Predict(this.vw, example.Ptr);
    //    }

    //    public void Dispose()
    //    {
    //        this.Dispose(true);
    //        GC.SuppressFinalize(this);
    //    }

    //    private void Dispose(bool disposing)
    //    {
    //        // Free unmanaged resources
    //        if (this.vw != IntPtr.Zero)
    //        {
    //            VowpalWabbitInterface.Finish(this.vw);
    //            this.vw = IntPtr.Zero;
    //        }
    //    }
    //}
    
    public class VowpalWabbit<TExample> : VowpalWabbit
    {
        protected readonly Func<TExample, VowpalWabbitInterfaceVisitor, VowpalWabbitInterfaceExample> serializer;
        protected readonly VowpalWabbitInterfaceVisitor visitor;

        public VowpalWabbit(string arguments) : base(arguments)
        {
            this.visitor = new VowpalWabbitInterfaceVisitor(this);

            // Compile serializer
            this.serializer = VowpalWabbitSerializer.CreateNativeSerializer<TExample>();
        }
        
        public float GetCostSensitivePrediction(TExample example)
        {
            // TODO: support action dependent features
            using (var vwExample = this.serializer(example, this.visitor))
            {
                this.Predict(vwExample);

                return VowpalWabbitInterface.GetCostSensitivePrediction(vwExample.Ptr);
            }
        }

        //public void Learn(TExample example, string label)
        //{
        //    using (var vwExample = this.serializer(example, this.visitor))
        //    {
        //        vwExample.ImportLabeledInto(this, label);
        //        this.Learn(vwExample);
        //    }
        //}

        public IVowpalWabbitExample ReadExample(TExample example)
        {
            return this.serializer(example, this.visitor);
        }
    }

    public sealed class VowpalWabbit<TExample, TActionDependentFeature> : VowpalWabbit<TExample>
        where TExample : IActionDependentFeatureExample<TActionDependentFeature>
    {
        private readonly Func<TActionDependentFeature, VowpalWabbitInterfaceVisitor, VowpalWabbitInterfaceExample> actionDependentFeatureSerializer;
        private readonly VowpalWabbitInterfaceVisitor actionDependentFeatureVisitor;

        public VowpalWabbit(string arguments) : base(arguments)
        {
            this.actionDependentFeatureVisitor = new VowpalWabbitInterfaceVisitor(this);
            this.actionDependentFeatureSerializer = VowpalWabbitSerializer.CreateNativeSerializer<TActionDependentFeature>();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="example"></param>
        /// <param name="chosenAction">Must be an an instance out of example.ActionDependentFeatures.</param>
        /// <param name="cost"></param>
        /// <param name="probability"></param>
        public void MSNTrain(TExample example, TActionDependentFeature chosenAction, float cost, float probability)
        {
            // TODO: where to stick the cost
            // shared|userlda: .1
            // `doc1|lda :.1 :.2
            // 0:cost:prob `doc2 |lda :.2 :.3
            // <new line>

            var examples = new List<VowpalWabbitInterfaceExample>();
            
            try
            {
                var sharedExample = this.serializer(example, this.visitor);
                examples.Add(sharedExample);

                if (!sharedExample.IsEmpty)
                {
                    sharedExample.AddLabel("shared");
                    this.Learn(sharedExample);
                }

                // leave as loop so if the serializer throws an exception, anything allocated so far can be free'd
                foreach (var actionDependentFeature in example.ActionDependentFeatures)
                {
                    // TODO: insert caching here
                    var adfExample = this.actionDependentFeatureSerializer(actionDependentFeature, this.visitor);
                    examples.Add(adfExample);

                    if (adfExample.IsEmpty)
                    {
                        continue;
                    }

                    if (object.ReferenceEquals(actionDependentFeature, chosenAction))
                    {
                        adfExample.AddLabel(
                            string.Format(
                            CultureInfo.InvariantCulture, 
                            "0:{1}:{2}",
                            cost, probability));
                    }

                    this.Learn(adfExample);
                }

                // allocate empty example to signal we're finished
                var finalExample = new VowpalWabbitInterfaceExample(this, new VowpalWabbitInterface.FEATURE_SPACE[0], new GCHandle[0]);

                this.Learn(finalExample);
            }
            finally
            {
                foreach (var e in examples)
                {
                    e.Dispose();
                }
            }
        }

        public TActionDependentFeature[] MSNPredict(TExample example)
        {
            // shared |userlda :.1 |che a:.1 
            // `doc1 |lda :.1 :.2 [1]
            // `doc2 |lda :.2 :.3 [2]
            // <new line>
            var examples = new List<VowpalWabbitInterfaceExample>();
            
            try
            {
                var sharedExample = this.serializer(example, this.visitor);
                examples.Add(sharedExample);

                if (!sharedExample.IsEmpty)
                {
                    sharedExample.AddLabel("shared");
                    this.Predict(sharedExample);
                }

                // leave as loop so if the serializer throws an exception, anything allocated so far can be free'd
                foreach (var actionDependentFeature in example.ActionDependentFeatures)
                {
                    // TODO: insert caching here
                    var adfExample = this.actionDependentFeatureSerializer(actionDependentFeature, this.visitor);
                    examples.Add(adfExample);

                    if (adfExample.IsEmpty)
                    {
                        continue;
                    }

                    this.Predict(adfExample);
                }

                // allocate empty example to signal we're finished
                var finalExample = new VowpalWabbitInterfaceExample(this, new VowpalWabbitInterface.FEATURE_SPACE[0], new GCHandle[0]);

                this.Predict(finalExample);

                // no need to free labelsPtr (managed by finalExample through VW)
                var labelCount = IntPtr.Zero;
                var labelsPtr = VowpalWabbitInterface.GetMultilabelPredictions(this.vw, finalExample.Ptr, ref labelCount);
                var multiLabelPrediction = new int[(int)labelCount];
                Marshal.Copy(labelsPtr, multiLabelPrediction, 0, multiLabelPrediction.Length);

                // re-shuffle
                var result = new TActionDependentFeature[multiLabelPrediction.Length];
                for (var i = 0; i < multiLabelPrediction.Length; i++)
			    {
                    // VW indicies are 1-based
			        result[i] = example.ActionDependentFeatures[multiLabelPrediction[i] - 1];
			    }

                return result;
            }
            finally
            {
                foreach (var e in examples)
                {
                    e.Dispose();
                }
            }
        }
    }

    public sealed class VowpalWabbitString<TExample> : VowpalWabbit
    {
        private readonly Func<TExample, VowpalWabbitStringVisitor, string> serializer;
        private readonly VowpalWabbitStringVisitor stringVisitor;

        public VowpalWabbitString(string arguments)
            : base(arguments)
        {
            // Compile serializer
            this.stringVisitor = new VowpalWabbitStringVisitor();
            this.serializer = VowpalWabbitSerializer.CreateStringSerializer<TExample>();
        }
        
        public IVowpalWabbitExample ReadExample(TExample example)
        {
            var exampleLine = this.serializer(example, this.stringVisitor);

            return base.ReadExample(exampleLine);
        }
    }
}

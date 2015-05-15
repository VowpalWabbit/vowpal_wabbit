using Microsoft.Research.MachineLearning.Serializer;
using Microsoft.Research.MachineLearning.Serializer.Visitors;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using Microsoft.Research.MachineLearning.Serializer.Attributes;

namespace Microsoft.Research.MachineLearning
{
    public class VowpalWabbit<TExample> : VowpalWabbit
    {
        protected readonly VowpalWabbitSerializer<TExample, VowpalWabbitExample> serializer;

        public VowpalWabbit(VowpalWabbitModel model)
            : base(model)
        {
            var visitor = new VowpalWabbitInterfaceVisitor(this);
            this.serializer = VowpalWabbitSerializerFactory.CreateSerializer<TExample>(visitor);
        }

        public VowpalWabbit(string arguments) : base(arguments)
        {
            var visitor = new VowpalWabbitInterfaceVisitor(this);
            this.serializer = VowpalWabbitSerializerFactory.CreateSerializer<TExample>(visitor);
        }
        
        public VowpalWabbitExample ReadExample(TExample example)
        {
            return this.serializer.Serialize(example);    
        }
    }

    public sealed class VowpalWabbit<TExample, TActionDependentFeature> : VowpalWabbit<TExample>
        where TExample : IActionDependentFeatureExample<TActionDependentFeature>
    {
        private readonly VowpalWabbitSerializer<TActionDependentFeature, VowpalWabbitExample> actionDependentFeatureSerializer;

        public VowpalWabbit(VowpalWabbitModel model) : base(model)
        {
            var visitor = new VowpalWabbitInterfaceVisitor(this);
            this.actionDependentFeatureSerializer = VowpalWabbitSerializerFactory.CreateSerializer<TActionDependentFeature>(visitor);
        }

        public VowpalWabbit(string arguments) : base(arguments)
        {
            var visitor = new VowpalWabbitInterfaceVisitor(this);
            this.actionDependentFeatureSerializer = VowpalWabbitSerializerFactory.CreateSerializer<TActionDependentFeature>(visitor);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="example"></param>
        /// <param name="chosenAction">Must be an an instance out of example.ActionDependentFeatures.</param>
        /// <param name="cost"></param>
        /// <param name="probability"></param>
        public float Train(TExample example, TActionDependentFeature chosenAction, float cost, float probability)
        {
            var examples = new List<VowpalWabbitExample>();
            
            try
            {
                var sharedExample = this.serializer.Serialize(example);
                examples.Add(sharedExample);

                if (!sharedExample.IsEmpty)
                {
                    sharedExample.AddLabel("shared");
                    sharedExample.Learn();
                }

                // leave as loop so if the serializer throws an exception, anything allocated so far can be free'd
                foreach (var actionDependentFeature in example.ActionDependentFeatures)
                {
                    // TODO: insert caching here
                    var adfExample = this.actionDependentFeatureSerializer.Serialize(actionDependentFeature);
                    examples.Add(adfExample);

                    // TODO: this is broken as IsEmpty is only true in the CreateEmptyExample case. Not sure
                    // how to determine if an action dependent feature example is empty and thus mis identified as a 
                    // final example
                    if (adfExample.IsEmpty)
                    {
                        continue;
                    }

                    if (object.ReferenceEquals(actionDependentFeature, chosenAction))
                    {
                        adfExample.AddLabel(
                            string.Format(
                            CultureInfo.InvariantCulture, 
                            "0:{0}:{1}",
                            cost, probability));
                    }

                    adfExample.Learn();
                }

                // allocate empty example to signal we're finished
                var finalExample = this.CreateEmptyExample();
                examples.Add(finalExample);

                return finalExample.Learn();
            }
            finally
            {
                foreach (var e in examples)
                {
                    e.Dispose();
                }
            }
        }

        public TActionDependentFeature[] Predict(TExample example)
        {
            // shared |userlda :.1 |che a:.1 
            // `doc1 |lda :.1 :.2 [1]
            // `doc2 |lda :.2 :.3 [2]
            // <new line>
            var examples = new List<VowpalWabbitExample>();
            
            try
            {
                var sharedExample = this.serializer.Serialize(example);
                examples.Add(sharedExample);

                if (!sharedExample.IsEmpty)
                {
                    sharedExample.AddLabel("shared");
                    sharedExample.Predict();
                }

                // leave as loop so if the serializer throws an exception, anything allocated so far can be free'd
                foreach (var actionDependentFeature in example.ActionDependentFeatures)
                {
                    var adfExample = this.actionDependentFeatureSerializer.Serialize(actionDependentFeature);
                    examples.Add(adfExample);

                    if (adfExample.IsEmpty)
                    {
                        continue;
                    }

                    adfExample.Predict();
                }

                // allocate empty example to signal we're finished
                var finalExample = this.CreateEmptyExample();
                examples.Add(finalExample);

                finalExample.Predict();

                var multiLabelPrediction = finalExample.MultilabelPredictions;

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

    /// <summary>
    /// Useful for debugging
    /// </summary>
    public sealed class VowpalWabbitString<TExample> : VowpalWabbit
    {
        private readonly VowpalWabbitSerializer<TExample, string> serializer;
        private readonly VowpalWabbitStringVisitor stringVisitor;

        public VowpalWabbitString(string arguments)
            : base(arguments)
        {
            // Compile serializer
            this.stringVisitor = new VowpalWabbitStringVisitor();
            this.serializer = VowpalWabbitSerializerFactory.CreateSerializer<TExample>(this.stringVisitor);
        }
        
        public VowpalWabbitExample ReadExample(TExample example)
        {
            var exampleLine = this.serializer.Serialize(example);
            return base.ReadExample(exampleLine);
        }
    }
}

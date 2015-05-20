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
            if (this.serializer == null)
            {
                return null;
            }

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

            if (this.actionDependentFeatureSerializer == null)
            {
                throw new ArgumentException(typeof(TActionDependentFeature) + " must have a least a single [Feature] defined.");
            }
        }

        public VowpalWabbit(string arguments) : base(arguments)
        {
            var visitor = new VowpalWabbitInterfaceVisitor(this);
            this.actionDependentFeatureSerializer = VowpalWabbitSerializerFactory.CreateSerializer<TActionDependentFeature>(visitor);

            if (this.actionDependentFeatureSerializer == null)
            {
                throw new ArgumentException(typeof(TActionDependentFeature) + " must have a least a single [Feature] defined.");
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="example"></param>
        /// <param name="chosenAction">Must be an an instance out of example.ActionDependentFeatures.</param>
        /// <param name="cost"></param>
        /// <param name="probability"></param>
        public TActionDependentFeature[] Learn(TExample example, TActionDependentFeature chosenAction, float cost, float probability)
        {
            return this.LearnOrPredict(example, ex =>
            {
                if (object.ReferenceEquals(ex, chosenAction))
                {
                    ex.AddLabel(
                        string.Format(
                        CultureInfo.InvariantCulture,
                        "0:{0}:{1}",
                        cost, probability));
                }

                ex.Learn();
            });
        }

        public TActionDependentFeature[] Predict(TExample example)
        {
            return this.LearnOrPredict(example, ex => ex.Predict());
        }

        private TActionDependentFeature[] LearnOrPredict(TExample example, Action<VowpalWabbitExample> learnOrPredict)
        {
            // shared |userlda :.1 |che a:.1 
            // `doc1 |lda :.1 :.2 [1]
            // `doc2 |lda :.2 :.3 [2]
            // <new line>
            var examples = new List<VowpalWabbitExample>();
            
            try
            {
                VowpalWabbitExample firstExample = null;

                if (this.serializer != null)
                {
                    var sharedExample = this.serializer.Serialize(example);
                    examples.Add(sharedExample);

                    sharedExample.AddLabel("shared");
                    learnOrPredict(sharedExample);

                    firstExample = sharedExample;
                }

                // leave as loop so if the serializer throws an exception, anything allocated so far can be free'd
                foreach (var actionDependentFeature in example.ActionDependentFeatures)
                {
                    var adfExample = this.actionDependentFeatureSerializer.Serialize(actionDependentFeature);
                    examples.Add(adfExample);

                    if (adfExample.IsNewLine)
                    {
                        continue;
                    }

                    learnOrPredict(adfExample);

                    if (firstExample == null)
                    {
                        firstExample = adfExample;
                    }
                }

                // allocate empty example to signal we're finished
                var finalExample = this.CreateEmptyExample();
                examples.Add(finalExample);

                learnOrPredict(finalExample);

                var multiLabelPrediction = firstExample.MultilabelPredictions;

                // re-shuffle
                var result = new TActionDependentFeature[multiLabelPrediction.Length];
                for (var i = 0; i < multiLabelPrediction.Length; i++)
			    {
                    // VW indicies are 1-based
			        result[i] = example.ActionDependentFeatures[multiLabelPrediction[i]];
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

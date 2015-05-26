using Microsoft.Research.MachineLearning.Serializer;
using Microsoft.Research.MachineLearning.Serializer.Visitors;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using Microsoft.Research.MachineLearning.Serializer.Attributes;
using Microsoft.Research.MachineLearning.Interfaces;

namespace Microsoft.Research.MachineLearning
{
    public class VowpalWabbit<TExample> : VowpalWabbit
    {
        protected readonly VowpalWabbitSerializer<TExample> serializer;

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
        
        public IVowpalWabbitExample ReadExample(TExample example)
        {
            if (this.serializer == null)
            {
                return null;
            }

            return this.serializer.Serialize(example);    
        }

        protected override void Dispose(bool isDiposing)
        {
            base.Dispose(isDiposing);

            if (isDiposing)
            {
                if (this.serializer != null)
                {
                    this.serializer.Dispose();
                    this.serializer = null;
                }
            }
        }
    }

    public sealed class VowpalWabbit<TExample, TActionDependentFeature> : VowpalWabbit<TExample>
        where TExample : SharedExample, IActionDependentFeatureExample<TActionDependentFeature>
    {
        private readonly VowpalWabbitSerializer<TActionDependentFeature> actionDependentFeatureSerializer;

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

        public TActionDependentFeature[] Learn(TExample example)
        {
            return this.LearnOrPredict(example, ex => ex.Learn());
        }

        public TActionDependentFeature[] Predict(TExample example)
        {
            return this.LearnOrPredict(example, ex => ex.Predict());
        }

        private TActionDependentFeature[] LearnOrPredict(TExample example, Action<IVowpalWabbitExample> learnOrPredict)
        {
            // shared |userlda :.1 |che a:.1 
            // `doc1 |lda :.1 :.2 [1]
            // `doc2 |lda :.2 :.3 [2]
            // <new line>
            var examples = new List<IVowpalWabbitExample>();
            
            try
            {
                IVowpalWabbitExample firstExample = null;

                if (this.serializer != null)
                {
                    var sharedExample = this.serializer.Serialize(example);
                    examples.Add(sharedExample);

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
                    // VW multi-label indicies are 0-based
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

        protected override void Dispose(bool isDiposing)
        {
            base.Dispose(isDiposing);

            if (isDiposing)
            {
                if (this.actionDependentFeatureSerializer != null)
                {
                    this.actionDependentFeatureSerializer.Dispose();
                    this.actionDependentFeatureSerializer = null;
                }
            }
        }
    }

    /// <summary>
    /// Useful for debugging
    /// </summary>
    public sealed class VowpalWabbitString<TExample> : VowpalWabbit
    {
        //private readonly VowpalWabbitSerializer<TExample, string> serializer;
        private readonly VowpalWabbitStringVisitor stringVisitor;

        public VowpalWabbitString(string arguments)
            : base(arguments)
        {
            // Compile serializer
            this.stringVisitor = new VowpalWabbitStringVisitor();
            //this.serializer = VowpalWabbitSerializerFactory.CreateSerializer<TExample>(this.stringVisitor);
        }
        
        public VowpalWabbitExample ReadExample(TExample example)
        {
            //var exampleLine = this.serializer.Serialize(example);
            //return base.ReadExample(exampleLine);
            return null;
        }
    }
}

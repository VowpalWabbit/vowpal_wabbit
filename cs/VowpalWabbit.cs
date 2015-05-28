// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbit.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using Microsoft.Research.MachineLearning.Interfaces;
using Microsoft.Research.MachineLearning.Serializer;
using Microsoft.Research.MachineLearning.Serializer.Visitors;

namespace Microsoft.Research.MachineLearning
{
    /// <summary>
    /// A wrapper for Vowpal Wabbit using a native serializer transferring data using the library interface.
    /// </summary>
    /// <typeparam name="TExample">The user example type.</typeparam>
    public class VowpalWabbit<TExample> : VowpalWabbit
    {
        protected VowpalWabbitSerializer<TExample> serializer;

        public VowpalWabbit(VowpalWabbitModel model, int maxExampleCacheSize = int.MaxValue)
            : base(model)
        {
            var visitor = new VowpalWabbitInterfaceVisitor(this);
            this.serializer = VowpalWabbitSerializerFactory.CreateSerializer<TExample>(visitor, maxExampleCacheSize);
        }

        public VowpalWabbit(string arguments, int maxExampleCacheSize = int.MaxValue)
            : base(arguments)
        {
            var visitor = new VowpalWabbitInterfaceVisitor(this);
            this.serializer = VowpalWabbitSerializerFactory.CreateSerializer<TExample>(visitor, maxExampleCacheSize);
        }
        
        /// <summary>
        /// Serializes <paramref name="example"/> into VowpalWabbit.
        /// </summary>
        /// <param name="example">The example to be read.</param>
        /// <returns>A native Vowpal Wabbit representation of the example.</returns>
        public IVowpalWabbitExample ReadExample(TExample example)
        {
            return this.serializer.Serialize(example);    
        }

        protected override void Dispose(bool isDiposing)
        {
            base.Dispose(isDiposing);

            if (isDiposing)
            {
                if (this.serializer != null)
                {
                    // free cached examples
                    this.serializer.Dispose();
                    this.serializer = null;
                }
            }
        }
    }

    /// <summary>
    /// A wrapper around Vowpal Wabbit to simplify action dependent feature scenarios.
    /// </summary>
    /// <typeparam name="TExample">The user example type.</typeparam>
    /// <typeparam name="TActionDependentFeature">The user action dependent feature type.</typeparam>
    public sealed class VowpalWabbit<TExample, TActionDependentFeature> : VowpalWabbit<TExample>
        where TExample : SharedExample, IActionDependentFeatureExample<TActionDependentFeature>
    {
        private VowpalWabbitSerializer<TActionDependentFeature> actionDependentFeatureSerializer;

        public VowpalWabbit(VowpalWabbitModel model, int maxExampleCacheSize = int.MaxValue)
            : base(model)
        {
            var visitor = new VowpalWabbitInterfaceVisitor(this);
            this.actionDependentFeatureSerializer = VowpalWabbitSerializerFactory.CreateSerializer<TActionDependentFeature>(visitor, maxExampleCacheSize);

            if (this.actionDependentFeatureSerializer == null)
            {
                throw new ArgumentException(typeof(TActionDependentFeature) + " must have a least a single [Feature] defined.");
            }
        }

        public VowpalWabbit(string arguments, int maxExampleCacheSize = int.MaxValue)
            : base(arguments)
        {
            var visitor = new VowpalWabbitInterfaceVisitor(this);
            this.actionDependentFeatureSerializer = VowpalWabbitSerializerFactory.CreateSerializer<TActionDependentFeature>(visitor, maxExampleCacheSize);

            if (this.actionDependentFeatureSerializer == null)
            {
                throw new ArgumentException(typeof(TActionDependentFeature) + " must have a least a single [Feature] defined.");
            }
        }

        /// <summary>
        /// Simplify learning of examples with action dependent features. 
        /// </summary>
        /// <param name="example">The user example</param>
        /// <returns>An ordered subset of predicted action dependent features.</returns>
        public TActionDependentFeature[] Learn(TExample example)
        {
            return this.LearnOrPredict(example, ex => ex.Learn());
        }

        /// <summary>
        /// Simplify prediction of examples with action dependent features.
        /// </summary>
        /// <param name="example">The user example.</param>
        /// <returns>An ordered subset of predicted action dependent features.</returns>
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
                // contains prediction results
                var sharedExample = this.serializer.Serialize(example);

                // check if we have shared features
                if (sharedExample != null)
                {
                    examples.Add(sharedExample);
                    learnOrPredict(sharedExample);
                    sharedExample.Finish();
                }

                // leave as loop (vs. linq) so if the serializer throws an exception, anything allocated so far can be free'd
                foreach (var actionDependentFeature in example.ActionDependentFeatures)
                {
                    var adfExample = this.actionDependentFeatureSerializer.Serialize(actionDependentFeature);
                    examples.Add(adfExample);

                    learnOrPredict(adfExample);
                    adfExample.Finish();
                }

                // allocate empty example to signal we're finished
                var finalExample = this.CreateEmptyExample();
                examples.Add(finalExample);

                learnOrPredict(finalExample);
                finalExample.Finish();

                var multiLabelPrediction = examples.First().MultilabelPredictions;

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
                // dispose examples
                // Note: most not dispose examples before final example
                // as the learning algorithm (such as cbf) keeps a reference 
                // to the example
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
}

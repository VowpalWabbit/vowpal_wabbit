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

        public VowpalWabbit(VowpalWabbitModel model, VowpalWabbitSerializerSettings settings = null)
            : base(model)
        {
            var visitor = new VowpalWabbitInterfaceVisitor(this);
            this.serializer = VowpalWabbitSerializerFactory.CreateSerializer<TExample>(visitor, settings);
        }

        public VowpalWabbit(string arguments, VowpalWabbitSerializerSettings settings = null)
            : base(arguments)
        {
            var visitor = new VowpalWabbitInterfaceVisitor(this);
            this.serializer = VowpalWabbitSerializerFactory.CreateSerializer<TExample>(visitor, settings);
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
            if (isDiposing)
            {
                if (this.serializer != null)
                {
                    // free cached examples
                    this.serializer.Dispose();
                    this.serializer = null;
                }
            }

            // don't dispose VW before we can dispose all cached examples
            base.Dispose(isDiposing);
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
        private VowpalWabbitExample emptyExample;

        public VowpalWabbit(VowpalWabbitModel model, VowpalWabbitSerializerSettings settings = null)
            : base(model)
        {
            this.Initialize(settings);
        }

        public VowpalWabbit(string arguments, VowpalWabbitSerializerSettings settings = null)
            : base(arguments)
        {
            this.Initialize(settings);
        }

        private void Initialize(VowpalWabbitSerializerSettings settings)
        {
            var visitor = new VowpalWabbitInterfaceVisitor(this);
            this.actionDependentFeatureSerializer = VowpalWabbitSerializerFactory.CreateSerializer<TActionDependentFeature>(visitor, settings);

            if (this.actionDependentFeatureSerializer == null)
            {
                throw new ArgumentException(typeof(TActionDependentFeature) + " must have a least a single [Feature] defined.");
            }

            using (var exBuilder = new VowpalWabbitExampleBuilder(this))
            {
                this.emptyExample = exBuilder.CreateExample();
            }
        }

        /// <summary>
        /// Simplify learning of examples with action dependent features. 
        /// </summary>
        /// <param name="example">The user example</param>
        /// <returns>An ordered subset of predicted action dependent features.</returns>
        public TActionDependentFeature[] Learn(TExample example)
        {
            return this.LearnOrPredict(example, predict:false);
        }

        /// <summary>
        /// Simplify prediction of examples with action dependent features.
        /// </summary>
        /// <param name="example">The user example.</param>
        /// <returns>An ordered subset of predicted action dependent features.</returns>
        public TActionDependentFeature[] Predict(TExample example)
        {
            return this.LearnOrPredict(example, predict:true);
        }

        /// <summary>
        /// Simplify learning of examples with action dependent features. 
        /// </summary>
        /// <param name="example">The user example</param>
        /// <returns>An ordered subset of predicted action indexes.</returns>
        public int[] LearnIndex<TPrediction>(TExample example)
        {
            return this.LearnOrPredictIndex(example, predict:false);
        }

        /// <summary>
        /// Simplify prediction of examples with action dependent features.
        /// </summary>
        /// <param name="example">The user example.</param>
        /// <returns>An ordered subset of predicted action indexes.</returns>
        public int[] PredictIndex(TExample example)
        {
            return this.LearnOrPredictIndex(example, predict: true);
        }

        private TActionDependentFeature[] LearnOrPredict(TExample example, bool predict)
        {
            int[] multiLabelPredictions = this.LearnOrPredictIndex(example, predict);

            // re-shuffle
            var result = new TActionDependentFeature[multiLabelPredictions.Length];
            for (var i = 0; i < multiLabelPredictions.Length; i++)
			{
                // VW multi-label indicies are 0-based
                result[i] = example.ActionDependentFeatures[multiLabelPredictions[i]];
			}

            return result;
        }

        private int[] LearnOrPredictIndex(TExample example, bool predict)
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
                    if (predict)
                        sharedExample.Learn<VowpalWabbitPredictionNone>();
                    else
                        sharedExample.Predict<VowpalWabbitPredictionNone>();
                }

                // leave as loop (vs. linq) so if the serializer throws an exception, anything allocated so far can be free'd
                foreach (var actionDependentFeature in example.ActionDependentFeatures)
                {
                    var adfExample = this.actionDependentFeatureSerializer.Serialize(actionDependentFeature);
                    examples.Add(adfExample);

                    if (predict)
                        adfExample.Learn<VowpalWabbitPredictionNone>();
                    else
                        adfExample.Predict<VowpalWabbitPredictionNone>();
                }

                // signal we're finished using an empty example
                this.emptyExample.Learn<VowpalWabbitPredictionNone>();

                // Nasty workaround. Since the prediction result is stored in the first example
                // and we'll have to get an actual VowpalWabbitExampt
                // 
                var firstExample = examples.FirstOrDefault();
                if (firstExample == null)
                {
                    return null;
                }

                var prediction = new VowpalWabbitMultilabelPrediction();
                prediction.ReadFromExample(firstExample.UnderlyingExample);
                
                return prediction.Values;
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
            if (isDiposing)
            {
                if (this.actionDependentFeatureSerializer != null)
                {
                    this.actionDependentFeatureSerializer.Dispose();
                    this.actionDependentFeatureSerializer = null;
                }
                if (this.emptyExample != null)
                {
                    this.emptyExample.Dispose();
                    this.emptyExample = null;
                }
            }
            base.Dispose(isDiposing);
        }
    }
}

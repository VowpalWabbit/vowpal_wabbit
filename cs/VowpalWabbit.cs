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
using VW.Interfaces;
using VW.Serializer;
using VW.Serializer.Visitors;

namespace VW
{
    /// <summary>
    /// A wrapper for Vowpal Wabbit using a native serializer transferring data using the library interface.
    /// </summary>
    /// <typeparam name="TExample">The user example type.</typeparam>
    public class VowpalWabbit<TExample> : VowpalWabbit
    {
        /// <summary>
        /// The serializer for the example user type.
        /// </summary>
        protected VowpalWabbitSerializer<TExample> serializer;

        /// <summary>
        /// Initializes a new <see cref="VowpalWabbit{TExample}"/> instance.
        /// </summary>
        /// <param name="model">The shared model.</param>
        /// <param name="settings">The serializer settings.</param>
        public VowpalWabbit(VowpalWabbitModel model, VowpalWabbitSerializerSettings settings = null)
            : base(model)
        {
            var visitor = new VowpalWabbitInterfaceVisitor(this);
            this.serializer = VowpalWabbitSerializerFactory.CreateSerializer<TExample>(visitor, settings);
        }

        /// <summary>
        /// Initializes a new <see cref="VowpalWabbit{TExample}"/> instance.
        /// </summary>
        /// <param name="arguments">Command line arguments</param>
        /// <param name="settings">The serializer settings.</param>
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

        /// <summary>
        /// Cleanup.
        /// </summary>
        /// <param name="isDiposing">See IDiposable pattern.</param>
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
    public class VowpalWabbitPredictor<TExample, TActionDependentFeature> : VowpalWabbit<TExample>
        where TExample : SharedExample, IActionDependentFeatureExample<TActionDependentFeature>
    {
        protected VowpalWabbitSerializer<TActionDependentFeature> actionDependentFeatureSerializer;
        protected VowpalWabbitExample emptyExample;

        /// <summary>
        /// Initializes a new <see cref="VowpalWabbitPredictor{TExample,TActionDependentFeature}"/> instance.
        /// </summary>
        /// <param name="model">The shared model.</param>
        /// <param name="settings">The serializer settings.</param>
        public VowpalWabbitPredictor(VowpalWabbitModel model, VowpalWabbitSerializerSettings settings = null)
            : base(model)
        {
            this.Initialize(settings);
        }

        /// <summary>
        /// Initializes a new <see cref="VowpalWabbitPredictor{TExample,TActionDependentFeature}"/> instance.
        /// </summary>
        /// <param name="arguments">Command line arguments</param>
        /// <param name="settings">The serializer settings.</param>
        public VowpalWabbitPredictor(string arguments, VowpalWabbitSerializerSettings settings = null)
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
        /// Simplify prediction of examples with action dependent features.
        /// </summary>
        /// <param name="example">The user example.</param>
        /// <returns>An ordered subset of predicted action dependent features.</returns>
        public TActionDependentFeature[] Predict(TExample example)
        {
            var multiLabelPredictions = this.PredictIndex(example);

            return ReShuffle(example, multiLabelPredictions);
        }

        /// <summary>
        /// Reshuffles the the action dependent features based on indices returned by native space.
        /// </summary>
        /// <param name="example">The example used for prediction.</param>
        /// <param name="multiLabelPredictions">The indices used to reshuffle.</param>
        /// <returns>The action dependent features ordered by <paramref name="multiLabelPredictions"/></returns>
        protected static TActionDependentFeature[] ReShuffle(TExample example, int[] multiLabelPredictions)
        {
            // re-shuffle
            var result = new TActionDependentFeature[multiLabelPredictions.Length];
            for (var i = 0; i < multiLabelPredictions.Length; i++)
			{
                // VW multi-label indicies are 0-based
                result[i] = example.ActionDependentFeatures[multiLabelPredictions[i]];
			}

            return result;
        }

        /// <summary>
        /// Simplify prediction of examples with action dependent features.
        /// </summary>
        /// <param name="example">The user example.</param>
        /// <returns>An ordered subset of predicted action indexes.</returns>
        public int[] PredictIndex(TExample example)
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
                    sharedExample.PredictAndDiscard();
                }

                // leave as loop (vs. linq) so if the serializer throws an exception, anything allocated so far can be free'd
                foreach (var actionDependentFeature in example.ActionDependentFeatures)
                {
                    var adfExample = this.actionDependentFeatureSerializer.Serialize(actionDependentFeature);
                    examples.Add(adfExample);

                    adfExample.PredictAndDiscard();
                }

                // signal we're finished using an empty example
                this.emptyExample.PredictAndDiscard();

                // Nasty workaround. Since the prediction result is stored in the first example
                // and we'll have to get an actual VowpalWabbitExampt
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
                // Note: must not dispose examples before final example
                // as the learning algorithm (such as cbf) keeps a reference 
                // to the example
                foreach (var e in examples)
                {
                    e.Dispose();
                }
            }
        }

        /// <summary>
        /// Cleanup.
        /// </summary>
        /// <param name="isDiposing">See IDiposable pattern.</param>
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
    /// <summary>
    /// A wrapper around Vowpal Wabbit to simplify action dependent feature scenarios.
    /// </summary>
    /// <typeparam name="TExample">The user example type.</typeparam>
    /// <typeparam name="TActionDependentFeature">The user action dependent feature type.</typeparam>
    public sealed class VowpalWabbit<TExample, TActionDependentFeature> : VowpalWabbitPredictor<TExample, TActionDependentFeature>
        where TExample : SharedExample, IActionDependentFeatureExample<TActionDependentFeature>
        where TActionDependentFeature : IExample
    {
        /// <summary>
        /// Initializes a new <see cref="VowpalWabbit{TExample,TActionDependentFeature}"/> instance.
        /// </summary>
        /// <param name="model">The shared model.</param>
        /// <param name="settings">The serializer settings.</param>
        public VowpalWabbit(VowpalWabbitModel model, VowpalWabbitSerializerSettings settings = null)
            : base(model, settings)
        {
        }

        /// <summary>
        /// Initializes a new <see cref="VowpalWabbit{TExample,TActionDependentFeature}"/> instance.
        /// </summary>
        /// <param name="arguments">Command line arguments</param>
        /// <param name="settings">The serializer settings.</param>
        public VowpalWabbit(string arguments, VowpalWabbitSerializerSettings settings = null)
            : base(arguments, settings)
        {
        }

        /// <summary>
        /// Simplify learning of examples with action dependent features. 
        /// </summary>
        /// <param name="example">The user example.</param>
        public void Learn(TExample example)
        {
            var examples = new List<IVowpalWabbitExample>();

            try
            {
                // contains prediction results
                var sharedExample = this.serializer.Serialize(example);
                // check if we have shared features
                if (sharedExample != null)
                {
                    examples.Add(sharedExample);
                    sharedExample.Learn();
                }

                // leave as loop (vs. linq) so if the serializer throws an exception, anything allocated so far can be free'd
                foreach (var actionDependentFeature in example.ActionDependentFeatures)
                {
                    var adfExample = this.actionDependentFeatureSerializer.Serialize(actionDependentFeature);
                    examples.Add(adfExample);

                    adfExample.Learn();
                }

                // signal we're finished using an empty example
                this.emptyExample.Learn();
            }
            finally
            {
                // dispose examples
                // Note: must not dispose examples before final example
                // as the learning algorithm (such as cbf) keeps a reference 
                // to the example
                foreach (var e in examples)
                {
                    e.Dispose();
                }
            }
        }

        /// <summary>
        /// Simplify learning of examples with action dependent features. 
        /// </summary>
        /// <param name="example">The user example.</param>
        /// <returns>An ordered subset of predicted action dependent features.</returns>
        public TActionDependentFeature[] LearnAndPredict(TExample example)
        {
            var multiLabelPredictions = this.LearnAndPredictIndex(example);

            return ReShuffle(example, multiLabelPredictions);
        }

        /// <summary>
        /// Simplify learning of examples with action dependent features. 
        /// </summary>
        /// <param name="example">The user example.</param>
        /// <returns>An ordered subset of predicted action indexes.</returns>
        public int[] LearnAndPredictIndex(TExample example)
        {
            var examples = new List<IVowpalWabbitExample>();

            try
            {
                // contains prediction results
                var sharedExample = this.serializer.Serialize(example);
                // check if we have shared features
                if (sharedExample != null)
                {
                    examples.Add(sharedExample);
                    sharedExample.Learn();
                }

                // leave as loop (vs. linq) so if the serializer throws an exception, anything allocated so far can be free'd
                foreach (var actionDependentFeature in example.ActionDependentFeatures)
                {
                    var adfExample = this.actionDependentFeatureSerializer.Serialize(actionDependentFeature);
                    examples.Add(adfExample);

                    adfExample.Learn();
                }

                // signal we're finished using an empty example
                this.emptyExample.Learn();

                // Nasty workaround. Since the prediction result is stored in the first example
                // and we'll have to get an actual VowpalWabbitExampt
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
                // Note: must not dispose examples before final example
                // as the learning algorithm (such as cbf) keeps a reference 
                // to the example
                foreach (var e in examples)
                {
                    e.Dispose();
                }
            }
        }
    }
}

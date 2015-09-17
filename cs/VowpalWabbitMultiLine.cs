// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitMultiLine.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.Linq;
using VW.Interfaces;
using VW.Labels;
using VW.Serializer;

namespace VW
{
    /// <summary>
    /// Helper class to properly feed multi-line examples into vw.
    /// </summary>
    public static class VowpalWabbitMultiLine
    {
        /// <summary>
        /// Simplify learning of examples with action dependent features. 
        /// </summary>
        public static void Learn<TExample, TActionDependentFeature>(
            VowpalWabbit vw, 
            VowpalWabbitSerializer<TExample> serializer,
            VowpalWabbitSerializer<TActionDependentFeature> actionDependentFeatureSerializer,
            TExample example, 
            IEnumerable<TActionDependentFeature> actionDependentFeatures, 
            int index, 
            ILabel label)
        {
            Contract.Requires(vw != null);
            Contract.Requires(serializer != null);
            Contract.Requires(actionDependentFeatureSerializer != null);
            Contract.Requires(example != null);
            Contract.Requires(actionDependentFeatures != null);
            Contract.Requires(index >= 0);
            Contract.Requires(label != null);

#if DEBUG
            // only in debug, since it's a hot path
            if (actionDependentFeatureSerializer.CachesExamples)
            {
                throw new NotSupportedException("Cached examples cannot be used for learning");
            }
#endif

            var examples = new List<VowpalWabbitExample>();

            try
            {
                // contains prediction results
                var sharedExample = serializer.Serialize(vw, example, SharedLabel.Instance);
                // check if we have shared features
                if (sharedExample != null)
                {
                    examples.Add(sharedExample);
                    vw.Learn(sharedExample);
                }

                var i = 0;
                foreach (var actionDependentFeature in actionDependentFeatures)
                {
                    var adfExample = actionDependentFeatureSerializer.Serialize(vw, actionDependentFeature, i == index ? label : null);
                    Contract.Assert(adfExample != null);
                    
                    examples.Add(adfExample);

                    vw.Learn(adfExample);

                    i++;
                }

                // signal we're finished using an empty example
                var empty = vw.GetOrCreateEmptyExample();
                examples.Add(empty);
                vw.Learn(empty);

                // Dump input file for command line learning
                //File.AppendAllLines(@"c:\temp\msn.txt",
                //    examples.OfType<VowpalWabbitDebugExample>()
                //        .Select(e => e.VowpalWabbitString)
                //        .Union(new[] { "" }));
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
        /// <typeparam name="TExample">The type of the user example.</typeparam>
        /// <typeparam name="TActionDependentFeature">The type of the user action dependent features.</typeparam>
        /// <param name="vw">The vw instance.</param>
        /// <param name="serializer">The serializer for <typeparamref name="TExample"/>.</param>
        /// <param name="actionDependentFeatureSerializer">The serializer for <typeparamref name="TActionDependentFeature"/>.</param>
        /// <param name="example">The user example.</param>
        /// <param name="actionDependentFeatures">The action dependent features.</param>
        /// <param name="index">The index of action dependent feature to label.</param>
        /// <param name="label">The label for the selected action dependent feature.</param>
        /// <returns>An ranked subset of predicted action indexes.</returns>
        public static int[] LearnAndPredictIndex<TExample, TActionDependentFeature>(
            VowpalWabbit vw,
            VowpalWabbitSerializer<TExample> serializer,
            VowpalWabbitSerializer<TActionDependentFeature> actionDependentFeatureSerializer, 
            TExample example, 
            IEnumerable<TActionDependentFeature> actionDependentFeatures, 
            int index, 
            ILabel label)
        {
            Contract.Requires(vw != null);
            Contract.Requires(serializer != null);
            Contract.Requires(actionDependentFeatureSerializer != null);
            Contract.Requires(example != null);
            Contract.Requires(actionDependentFeatures != null);
            Contract.Requires(index >= 0);
            Contract.Requires(label != null);

#if DEBUG
            // only in debug, since it's a hot path
            if (actionDependentFeatureSerializer.CachesExamples)
            {
                throw new NotSupportedException("Cached examples cannot be used for learning");
            }
#endif

            var examples = new List<VowpalWabbitExample>();

            try
            {
                // contains prediction results
                var sharedExample = serializer.Serialize(vw, example);
                // check if we have shared features
                if (sharedExample != null)
                {
                    examples.Add(sharedExample);
                    vw.Learn(sharedExample);
                }

                // leave as loop (vs. linq) so if the serializer throws an exception, anything allocated so far can be free'd
                var i = 0;
                foreach (var actionDependentFeature in actionDependentFeatures)
                {
                    var adfExample = actionDependentFeatureSerializer.Serialize(vw, actionDependentFeature, i == index ? label : null);
                    Contract.Assert(adfExample != null);

                    examples.Add(adfExample);

                    vw.Learn(adfExample);
                    i++;
                }

                // signal we're finished using an empty example
                var empty = vw.GetOrCreateEmptyExample();
                examples.Add(empty);
                vw.Learn(empty);

                // Nasty workaround. Since the prediction result is stored in the first example
                // and we'll have to get an actual VowpalWabbitExampt
                var firstExample = examples.FirstOrDefault();
                if (firstExample == null)
                {
                    return null;
                }

                return firstExample.GetPrediction(vw, VowpalWabbitPredictionType.Multilabel);
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
        /// <typeparam name="TExample">The type of the user example.</typeparam>
        /// <typeparam name="TActionDependentFeature">The type of the user action dependent features.</typeparam>
        /// <param name="vw">The vw instance.</param>
        /// <param name="serializer">The serializer for <typeparamref name="TExample"/>.</param>
        /// <param name="actionDependentFeatureSerializer">The serializer for <typeparamref name="TActionDependentFeature"/>.</param>
        /// <param name="example">The user example.</param>
        /// <param name="actionDependentFeatures">The action dependent features.</param>
        /// <param name="index">The index of action dependent feature to label.</param>
        /// <param name="label">The label for the selected action dependent feature.</param>
        /// <returns>An ranked subset of predicted actions.</returns>
        public static TActionDependentFeature[] LearnAndPredict<TExample, TActionDependentFeature>(
            VowpalWabbit vw,
            VowpalWabbitSerializer<TExample> serializer,
            VowpalWabbitSerializer<TActionDependentFeature> actionDependentFeatureSerializer,
            TExample example, 
            IEnumerable<TActionDependentFeature> actionDependentFeatures, 
            int index, 
            ILabel label)
        {
            Contract.Requires(vw != null);
            Contract.Requires(serializer != null);
            Contract.Requires(actionDependentFeatureSerializer != null);
            Contract.Requires(example != null);
            Contract.Requires(actionDependentFeatures != null);
            Contract.Requires(index >= 0);
            Contract.Requires(label != null);

#if DEBUG
            // only in debug, since it's a hot path
            if (actionDependentFeatureSerializer.CachesExamples)
            {
                throw new NotSupportedException("Cached examples cannot be used for learning");
            }
#endif

            var multiLabelPredictions = LearnAndPredictIndex(vw, serializer, actionDependentFeatureSerializer, example, actionDependentFeatures, index, label);
            return actionDependentFeatures.Subset(multiLabelPredictions);
        }

        /// <summary>
        /// Simplify prediction of examples with action dependent features.
        /// </summary>
        /// <typeparam name="TExample">The type of the user example.</typeparam>
        /// <typeparam name="TActionDependentFeature">The type of the user action dependent features.</typeparam>
        /// <param name="vw">The vw instance.</param>
        /// <param name="serializer">The serializer for <typeparamref name="TExample"/>.</param>
        /// <param name="actionDependentFeatureSerializer">The serializer for <typeparamref name="TActionDependentFeature"/>.</param>
        /// <param name="example">The user example.</param>
        /// <param name="actionDependentFeatures">The action dependent features.</param>
        /// <returns>An ranked subset of predicted action indexes.</returns>
        public static int[] PredictIndex<TExample, TActionDependentFeature>(
            VowpalWabbit vw,
            VowpalWabbitSerializer<TExample> serializer,
            VowpalWabbitSerializer<TActionDependentFeature> actionDependentFeatureSerializer, 
            TExample example, 
            IEnumerable<TActionDependentFeature> actionDependentFeatures)
        {
            Contract.Requires(vw != null);
            Contract.Requires(serializer != null);
            Contract.Requires(actionDependentFeatureSerializer != null);
            Contract.Requires(example != null);
            Contract.Requires(actionDependentFeatures != null);

            // shared |userlda :.1 |che a:.1 
            // `doc1 |lda :.1 :.2 [1]
            // `doc2 |lda :.2 :.3 [2]
            // <new line>
            var examples = new List<VowpalWabbitExample>();

            try
            {
                // contains prediction results
                var sharedExample = serializer.Serialize(vw, example);
                // check if we have shared features
                if (sharedExample != null)
                {
                    examples.Add(sharedExample);
                    vw.Predict(sharedExample);
                }

                // leave as loop (vs. linq) so if the serializer throws an exception, anything allocated so far can be free'd
                foreach (var actionDependentFeature in actionDependentFeatures)
                {
                    var adfExample = actionDependentFeatureSerializer.Serialize(vw, actionDependentFeature);
                    Contract.Assert(adfExample != null);

                    examples.Add(adfExample);

                    vw.Predict(adfExample);
                }

                // signal we're finished using an empty example
                var empty = vw.GetOrCreateEmptyExample();
                examples.Add(empty);
                vw.Predict(empty);

                // Nasty workaround. Since the prediction result is stored in the first example
                // and we'll have to get an actual VowpalWabbitExampt
                var firstExample = examples.FirstOrDefault();
                if (firstExample == null)
                {
                    return null;
                }

                return firstExample.GetPrediction(vw, VowpalWabbitPredictionType.Multilabel);
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
        /// Simplify prediction of examples with action dependent features.
        /// </summary>
        /// <typeparam name="TExample">The type of the user example.</typeparam>
        /// <typeparam name="TActionDependentFeature">The type of the user action dependent features.</typeparam>
        /// <param name="vw">The vw instance.</param>
        /// <param name="serializer">The serializer for <typeparamref name="TExample"/>.</param>
        /// <param name="actionDependentFeatureSerializer">The serializer for <typeparamref name="TActionDependentFeature"/>.</param>
        /// <param name="example">The user example.</param>
        /// <param name="actionDependentFeatures">The action dependent features.</param>
        /// <returns>An ranked subset of predicted actions.</returns>
        public static TActionDependentFeature[] Predict<TExample, TActionDependentFeature>(
            VowpalWabbit vw,
            VowpalWabbitSerializer<TExample> serializer,
            VowpalWabbitSerializer<TActionDependentFeature> actionDependentFeatureSerializer, 
            TExample example,
            IEnumerable<TActionDependentFeature> actionDependentFeatures)
        {
            Contract.Requires(vw != null);
            Contract.Requires(serializer != null);
            Contract.Requires(actionDependentFeatureSerializer != null);
            Contract.Requires(example != null);
            Contract.Requires(actionDependentFeatures != null);

            var multiLabelPredictions = PredictIndex(vw, serializer, actionDependentFeatureSerializer, example, actionDependentFeatures);
            return actionDependentFeatures.Subset(multiLabelPredictions);
        }
    }
}

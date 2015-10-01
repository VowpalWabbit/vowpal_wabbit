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
        public static void Execute<TExample, TActionDependentFeature>(
            VowpalWabbit vw,
            VowpalWabbitSerializer<TExample> serializer,
            VowpalWabbitSerializer<TActionDependentFeature> actionDependentFeatureSerializer,
            TExample example,
            IReadOnlyCollection<TActionDependentFeature> actionDependentFeatures,
            Action<List<VowpalWabbitExample>, List<TActionDependentFeature>> predictOrLearn,
            int? index = null,
            ILabel label = null)
        {
            Contract.Requires(vw != null);
            Contract.Requires(serializer != null);
            Contract.Requires(actionDependentFeatureSerializer != null);
            Contract.Requires(example != null);
            Contract.Requires(actionDependentFeatures != null);
            Contract.Requires(index >= 0);
            Contract.Requires(label != null);

            var examples = new List<VowpalWabbitExample>(actionDependentFeatures.Count + 1);
            var validExamples = new List<VowpalWabbitExample>(actionDependentFeatures.Count + 1);
            var validActionDependentFeatures = new List<TActionDependentFeature>(actionDependentFeatures.Count + 1);

            try
            {
                // contains prediction results
                var sharedExample = serializer.Serialize(example, SharedLabel.Instance);
                // check if we have shared features
                if (sharedExample != null)
                {
                    examples.Add(sharedExample);

                    if (!sharedExample.IsNewLine)
                    {
                        validExamples.Add(sharedExample);
                    }
                }

                var i = 0;
                foreach (var actionDependentFeature in actionDependentFeatures)
                {
                    var adfExample = actionDependentFeatureSerializer.Serialize(actionDependentFeature,
                        index != null && i == index ? label : null);
                    Contract.Assert(adfExample != null);

                    examples.Add(adfExample);

                    if (!adfExample.IsNewLine)
                    {
                        validExamples.Add(adfExample);
                        validActionDependentFeatures.Add(actionDependentFeature);
                    }

                    i++;
                }

                if (validActionDependentFeatures.Count == 0)
                {
                    return;
                }

                // signal we're finished using an empty example
                var empty = vw.GetOrCreateEmptyExample();
                examples.Add(empty);
                validExamples.Add(empty);

                predictOrLearn(validExamples, validActionDependentFeatures);
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
        public static void Learn<TExample, TActionDependentFeature>(
            VowpalWabbit vw,
            VowpalWabbitSerializer<TExample> serializer,
            VowpalWabbitSerializer<TActionDependentFeature> actionDependentFeatureSerializer,
            TExample example,
            IReadOnlyCollection<TActionDependentFeature> actionDependentFeatures,
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

            Execute(
                vw,
                serializer,
                actionDependentFeatureSerializer,
                example,
                actionDependentFeatures,
                (examples, _) =>
                {
                    foreach (var ex in examples)
                    {
                        vw.Learn(ex);
                    }
                },
                index,
                label);
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
            IReadOnlyCollection<TActionDependentFeature> actionDependentFeatures,
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

            int[] predictions = null;

            Execute(
                vw,
                serializer,
                actionDependentFeatureSerializer,
                example,
                actionDependentFeatures,
                (examples, validActionDependentFeatures) =>
                {
                    foreach (var ex in examples)
                    {
                        vw.Learn(ex);
                    }

                    predictions = VowpalWabbitMultiLine.GetPrediction(vw, actionDependentFeatures, examples);
                },
                index,
                label);

            // default to the input list
            return predictions ?? Enumerable.Range(0, actionDependentFeatures.Count).ToArray();
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
            IReadOnlyCollection<TActionDependentFeature> actionDependentFeatures,
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

            TActionDependentFeature[] predictions = null;

            Execute(
                vw,
                serializer,
                actionDependentFeatureSerializer,
                example,
                actionDependentFeatures,
                (examples, validActionDependentFeatures) =>
                {
                    foreach (var ex in examples)
                    {
                        vw.Learn(ex);
                    }

                    var indices = VowpalWabbitMultiLine.GetPrediction(vw, actionDependentFeatures, examples);

                    predictions = validActionDependentFeatures.Subset(indices);
                },
                index,
                label);

            // default to the input list
            return predictions ?? actionDependentFeatures.ToArray();
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
            IReadOnlyCollection<TActionDependentFeature> actionDependentFeatures,
            int? index = null,
            ILabel label = null)
        {
            Contract.Requires(vw != null);
            Contract.Requires(serializer != null);
            Contract.Requires(actionDependentFeatureSerializer != null);
            Contract.Requires(example != null);
            Contract.Requires(actionDependentFeatures != null);

            int[] predictions = null;

            Execute(
                vw,
                serializer,
                actionDependentFeatureSerializer,
                example,
                actionDependentFeatures,
                (examples, validActionDependentFeatures) =>
                {
                    foreach (var ex in examples)
                    {
                        vw.Predict(ex);
                    }

                    predictions = VowpalWabbitMultiLine.GetPrediction(vw, actionDependentFeatures, examples);
                },
                index,
                label);

            // default to the input list
            return predictions ?? Enumerable.Range(0, actionDependentFeatures.Count).ToArray();
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
            IReadOnlyCollection<TActionDependentFeature> actionDependentFeatures,
            int? index = null,
            ILabel label = null)
        {
            Contract.Requires(vw != null);
            Contract.Requires(serializer != null);
            Contract.Requires(actionDependentFeatureSerializer != null);
            Contract.Requires(example != null);
            Contract.Requires(actionDependentFeatures != null);

            TActionDependentFeature[] predictions = null;

            Execute(
                vw,
                serializer,
                actionDependentFeatureSerializer,
                example,
                actionDependentFeatures,
                (examples, validActionDependentFeatures) =>
                {
                    foreach (var ex in examples)
                    {
                        vw.Predict(ex);
                    }

                    var indices = VowpalWabbitMultiLine.GetPrediction(vw, actionDependentFeatures, examples);

                    predictions = validActionDependentFeatures.Subset(indices);
                },
                index,
                label);

            // default to the input list
            return predictions ?? actionDependentFeatures.ToArray();
        }

        public static int[] GetPrediction<TActionDependentFeature>(VowpalWabbit vw, IReadOnlyCollection<TActionDependentFeature> actionDependentFeatures, List<VowpalWabbitExample> examples)
        {
            // Nasty workaround. Since the prediction result is stored in the first example
            // and we'll have to get an actual VowpalWabbitExampt
            var firstExample = examples.FirstOrDefault();
            if (firstExample == null)
            {
                return Enumerable.Range(0, actionDependentFeatures.Count).ToArray();
            }

            var values = firstExample.GetPrediction(vw, VowpalWabbitPredictionType.Multilabel);

            if (values.Length == actionDependentFeatures.Count)
            {
                return values;
            }

            if (values.Length != examples.Count)
            {
                throw new InvalidOperationException("Number of predictions returned unequal number of examples fed");
            }

            // defaults to false
            var present = new bool[values.Length];
            // mark elements present in "bitset"
            foreach (var index in values)
            {
                present[index] = true;
            }

            // copy existing predictions to enlarged array
            var result = new int[actionDependentFeatures.Count];
            Array.Copy(values, result, values.Length);

            // append the ones that are not present in the prediction list
            var startIndex = values.Length;
            for (int i = 0; i < present.Length; i++)
            {
                if (!present[i])
                {
                    result[startIndex++] = i;
                }
            }

            return result;
        }
    }
}

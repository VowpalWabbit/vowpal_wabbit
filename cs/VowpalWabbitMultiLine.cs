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
            Action<List<VowpalWabbitExample>, List<Tuple<int, TActionDependentFeature>>, List<Tuple<int, TActionDependentFeature>>> predictOrLearn,
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
            var validActionDependentFeatures = new List<Tuple<int, TActionDependentFeature>>(actionDependentFeatures.Count + 1);
            var emptyActionDependentFeatures = new List<Tuple<int, TActionDependentFeature>>(actionDependentFeatures.Count + 1);

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
                        validActionDependentFeatures.Add(Tuple.Create(i, actionDependentFeature));
                    }
                    else
                    {
                        emptyActionDependentFeatures.Add(Tuple.Create(i, actionDependentFeature));
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

                predictOrLearn(validExamples, validActionDependentFeatures, emptyActionDependentFeatures);
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
                (examples, _, __) =>
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
        /// <returns>An ranked subset of predicted actions.</returns>
        public static Tuple<int, TActionDependentFeature>[] LearnAndPredict<TExample, TActionDependentFeature>(
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

            Tuple<int, TActionDependentFeature>[] predictions = null;

            Execute(
                vw,
                serializer,
                actionDependentFeatureSerializer,
                example,
                actionDependentFeatures,
                (examples, validActionDependentFeatures, emptyActionDependentFeatures) =>
                {
                    foreach (var ex in examples)
                    {
                        vw.Learn(ex);
                    }

                    predictions = VowpalWabbitMultiLine.GetPrediction(vw, examples, validActionDependentFeatures, emptyActionDependentFeatures);
                },
                index,
                label);

            // default to the input list
            return predictions ?? actionDependentFeatures.Select((o, i) => Tuple.Create(i, o)).ToArray();
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
        public static Tuple<int, TActionDependentFeature>[] Predict<TExample, TActionDependentFeature>(
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

            Tuple<int, TActionDependentFeature>[] predictions = null;

            Execute(
                vw,
                serializer,
                actionDependentFeatureSerializer,
                example,
                actionDependentFeatures,
                (examples, validActionDependentFeatures, emptyActionDependentFeatures) =>
                {
                    foreach (var ex in examples)
                    {
                        vw.Predict(ex);
                    }

                    predictions = VowpalWabbitMultiLine.GetPrediction(vw, examples, validActionDependentFeatures, emptyActionDependentFeatures);
                },
                index,
                label);

            // default to the input list
            return predictions ?? actionDependentFeatures.Select((o, i) => Tuple.Create(i, o)).ToArray();
        }

        public static Tuple<int, TActionDependentFeature>[] GetPrediction<TActionDependentFeature>(
            VowpalWabbit vw,
            List<VowpalWabbitExample> examples,
            List<Tuple<int, TActionDependentFeature>> validActionDependentFeatures,
            List<Tuple<int, TActionDependentFeature>> emptyActionDependentFeatures)
        {
            // Since the prediction result is stored in the first example
            // and we'll have to get an actual VowpalWabbitExampt
            var firstExample = examples.FirstOrDefault();
            if (firstExample == null)
            {
                return null;
            }

            var values = firstExample.GetPrediction(vw, VowpalWabbitPredictionType.Multilabel);

            if (values.Length != validActionDependentFeatures.Count)
            {
                throw new InvalidOperationException("Number of predictions returned unequal number of examples fed");
            }

            var result = new Tuple<int, TActionDependentFeature>[validActionDependentFeatures.Count + emptyActionDependentFeatures.Count];

            int i = 0;
            foreach (var index in values)
            {
                result[i++] = validActionDependentFeatures[index];
            }

            // append invalid ones at the end
            foreach (var f in emptyActionDependentFeatures)
            {
                result[i++] = f;
            }

            return result;
        }
    }
}

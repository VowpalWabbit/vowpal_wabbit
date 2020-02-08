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
using System.Text;
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
        /// Serializes the specifed example to VW native string format.
        /// </summary>
        /// <typeparam name="TExample">The user example type.</typeparam>
        /// <typeparam name="TActionDependentFeature">The user action dependent feature type.</typeparam>
        /// <param name="vw">The VW instance.</param>
        /// <param name="example">The shared example.</param>
        /// <param name="actionDependentFeatures">The action dependent features.</param>
        /// <param name="index">The optional index of the label example.</param>
        /// <param name="label">The optional label.</param>
        /// <param name="dictionary">Used to extract features into dictionary.</param>
        /// <param name="fastDictionary">Used to extract features into dictionary. This should use a faster comparison mehtod (e.g. reference equals).</param>
        /// <returns>The string serialized example.</returns>
        public static string SerializeToString<TExample, TActionDependentFeature>(
            VowpalWabbit<TExample, TActionDependentFeature> vw,
            TExample example,
            IReadOnlyCollection<TActionDependentFeature> actionDependentFeatures,
            int? index = null,
            ILabel label = null,
            Dictionary<string, string> dictionary = null,
            Dictionary<object, string> fastDictionary = null)
        {
#if DEBUG
            if (!vw.Native.Settings.EnableStringExampleGeneration)
            {
                throw new ArgumentException("vw.Settings.EnableStringExampleGeneration must be enabled");
            }
#endif

            return SerializeToString<TExample, TActionDependentFeature>(
                vw.Native,
                example,
                actionDependentFeatures,
                index,
                label,
                vw.ExampleSerializer,
                vw.ActionDependentFeatureSerializer,
                dictionary,
                fastDictionary);
        }

        /// <summary>
        /// Serializes the specifed example to VW native string format.
        /// </summary>
        /// <typeparam name="TExample">The user example type.</typeparam>
        /// <typeparam name="TActionDependentFeature">The user action dependent feature type.</typeparam>
        /// <param name="vw">The VW instance.</param>
        /// <param name="example">The shared example.</param>
        /// <param name="actionDependentFeatures">The action dependent features.</param>
        /// <param name="index">The optional index of the label example.</param>
        /// <param name="label">The optional label.</param>
        /// <param name="serializer">The example serializer.</param>
        /// <param name="actionDependentFeatureSerializer">The action dependent feature serializer.</param>
        /// <param name="dictionary">Dictionary used for dictify operation.</param>
        /// <param name="fastDictionary">Dictionary used for dictify operation.</param>
        /// <returns>The string serialized example.</returns>
        public static string SerializeToString<TExample, TActionDependentFeature>(
            VowpalWabbit vw,
            TExample example,
            IReadOnlyCollection<TActionDependentFeature> actionDependentFeatures,
            int? index = null,
            ILabel label = null,
            IVowpalWabbitSerializer<TExample> serializer = null,
            IVowpalWabbitSerializer<TActionDependentFeature> actionDependentFeatureSerializer = null,
            Dictionary<string, string> dictionary = null,
            Dictionary<object, string> fastDictionary = null)
        {
            if (vw == null)
                throw new ArgumentNullException("vw");

            if (serializer == null)
            {
                serializer = VowpalWabbitSerializerFactory.CreateSerializer<TExample>(new VowpalWabbitSettings { EnableStringExampleGeneration = true }).Create(vw);
            }
            else if (!serializer.EnableStringExampleGeneration)
            {
                throw new ArgumentException("Serializer must be compiled using EnableStringExampleGeneration = true");
            }

            if (actionDependentFeatureSerializer == null)
            {
                actionDependentFeatureSerializer = VowpalWabbitSerializerFactory.CreateSerializer<TActionDependentFeature>(new VowpalWabbitSettings { EnableStringExampleGeneration = true }).Create(vw);
            }
            else if (!actionDependentFeatureSerializer.EnableStringExampleGeneration)
            {
                throw new ArgumentException("Action dependent serializer must be compiled using EnableStringExampleGeneration = true");
            }

            var stringExample = new StringBuilder();

            var sharedExample = serializer.SerializeToString(example, SharedLabel.Instance, null, dictionary, fastDictionary);

            // check if we have shared features
            if (!string.IsNullOrWhiteSpace(sharedExample))
            {
                stringExample.AppendLine(sharedExample);
            }

            var i = 0;
            foreach (var actionDependentFeature in actionDependentFeatures)
            {
                var adfExample = actionDependentFeatureSerializer.SerializeToString(actionDependentFeature,
                    index != null && i == index ? label : null, null, dictionary, fastDictionary);

                if (!string.IsNullOrWhiteSpace(adfExample))
                {
                    stringExample.AppendLine(adfExample);
                }

                i++;
            }

            return stringExample.ToString();
        }

        /// <summary>
        /// A named delegate for the action to be taken once all the examples are marshalled.
        /// </summary>
        /// <typeparam name="TActionDependentFeature">The action dependent feature user type.</typeparam>
        /// <param name="validExamples">Marshalled valid examples.</param>
        /// <param name="validActionDependentFeatures">List of valid marshalled examples.</param>
        /// <param name="emptyActionDependentFeatures">List of empty non-marshalled examples.</param>
        public delegate void LearnOrPredictAction<TActionDependentFeature>(
            IReadOnlyList<VowpalWabbitExample> validExamples,
            IReadOnlyList<ActionDependentFeature<TActionDependentFeature>> validActionDependentFeatures,
            IReadOnlyList<ActionDependentFeature<TActionDependentFeature>> emptyActionDependentFeatures);

        /// <summary>
        /// Simplify learning of examples with action dependent features.
        /// </summary>
        /// <typeparam name="TExample">User example type.</typeparam>
        /// <typeparam name="TActionDependentFeature">Action dependent feature type.</typeparam>
        /// <param name="vw">The VowpalWabbit instances.</param>
        /// <param name="serializer">The example serializer.</param>
        /// <param name="actionDependentFeatureSerializer">The action dependent feature serializer.</param>
        /// <param name="example">The example.</param>
        /// <param name="actionDependentFeatures">The action dependent features.</param>
        /// <param name="predictOrLearn">An action executed once the set of valid examples is determined. </param>
        /// <param name="index">The optional index of the action dependent feature this label belongs too.</param>
        /// <param name="label">The optional label to be used for learning or evaluation.</param>
        public static void Execute<TExample, TActionDependentFeature>(
            VowpalWabbit vw,
            VowpalWabbitSingleExampleSerializer<TExample> serializer,
            VowpalWabbitSingleExampleSerializer<TActionDependentFeature> actionDependentFeatureSerializer,
            TExample example,
            IReadOnlyCollection<TActionDependentFeature> actionDependentFeatures,
            LearnOrPredictAction<TActionDependentFeature> predictOrLearn,
            int? index = null,
            ILabel label = null)
        {
            Contract.Requires(vw != null);
            Contract.Requires(actionDependentFeatureSerializer != null);
            Contract.Requires(example != null);
            Contract.Requires(actionDependentFeatures != null);

            var examples = new List<VowpalWabbitExample>(actionDependentFeatures.Count + 1);
            var validExamples = new List<VowpalWabbitExample>(actionDependentFeatures.Count + 1);
            var validActionDependentFeatures = new List<ActionDependentFeature<TActionDependentFeature>>(actionDependentFeatures.Count + 1);
            var emptyActionDependentFeatures = new List<ActionDependentFeature<TActionDependentFeature>>(actionDependentFeatures.Count + 1);

            VowpalWabbitExample emptyExample = null;

            try
            {
                // contains prediction results
                if (serializer != null)
                {
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
                        validActionDependentFeatures.Add(new ActionDependentFeature<TActionDependentFeature>(i, actionDependentFeature));
                    }
                    else
                    {
                        emptyActionDependentFeatures.Add(new ActionDependentFeature<TActionDependentFeature>(i, actionDependentFeature));
                    }

                    i++;
                }

                if (validActionDependentFeatures.Count == 0)
                    return;

                // signal we're finished using an empty example
                emptyExample = vw.GetOrCreateNativeExample();
                emptyExample.MakeEmpty(vw);

                predictOrLearn(validExamples, validActionDependentFeatures, emptyActionDependentFeatures);
            }
            finally
            {
                if (emptyExample != null)
                    emptyExample.Dispose();

                // dispose examples
                // Note: must not dispose examples before final example
                // as the learning algorithm (such as cbf) keeps a reference
                // to the example
                foreach (var e in examples)
                    e.Dispose();
            }
        }

        /// <summary>
        /// Simplify learning of examples with action dependent features.
        /// </summary>
        public static void Learn<TExample, TActionDependentFeature>(
            VowpalWabbit vw,
            VowpalWabbitSingleExampleSerializer<TExample> serializer,
            VowpalWabbitSingleExampleSerializer<TActionDependentFeature> actionDependentFeatureSerializer,
            TExample example,
            IReadOnlyCollection<TActionDependentFeature> actionDependentFeatures,
            int index,
            ILabel label)
        {
            Contract.Requires(vw != null);
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
                    vw.Learn(examples.ToList());
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
        public static ActionDependentFeature<TActionDependentFeature>[] LearnAndPredict<TExample, TActionDependentFeature>(
            VowpalWabbit vw,
            VowpalWabbitSingleExampleSerializer<TExample> serializer,
            VowpalWabbitSingleExampleSerializer<TActionDependentFeature> actionDependentFeatureSerializer,
            TExample example,
            IReadOnlyCollection<TActionDependentFeature> actionDependentFeatures,
            int index,
            ILabel label)
        {
            Contract.Requires(vw != null);
            Contract.Requires(actionDependentFeatureSerializer != null);
            Contract.Requires(example != null);
            Contract.Requires(actionDependentFeatures != null);
            Contract.Requires(index >= 0);
            Contract.Requires(label != null);

            ActionDependentFeature<TActionDependentFeature>[] predictions = null;

            Execute(
                vw,
                serializer,
                actionDependentFeatureSerializer,
                example,
                actionDependentFeatures,
                (examples, validActionDependentFeatures, emptyActionDependentFeatures) =>
                {
                    var ex_list = examples.ToList();
                    vw.Learn(ex_list);

                    predictions = VowpalWabbitMultiLine.GetPrediction(vw, examples, validActionDependentFeatures, emptyActionDependentFeatures);
                },
                index,
                label);

            // default to the input list
            return predictions ?? actionDependentFeatures.Select((o, i) => new ActionDependentFeature<TActionDependentFeature>(i, o)).ToArray();
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
        /// <param name="index">The index of action dependent feature to label.</param>
        /// <param name="label">The label for the selected action dependent feature.</param>
        /// <returns>An ranked subset of predicted actions.</returns>
        public static ActionDependentFeature<TActionDependentFeature>[] Predict<TExample, TActionDependentFeature>(
            VowpalWabbit vw,
            VowpalWabbitSingleExampleSerializer<TExample> serializer,
            VowpalWabbitSingleExampleSerializer<TActionDependentFeature> actionDependentFeatureSerializer,
            TExample example,
            IReadOnlyCollection<TActionDependentFeature> actionDependentFeatures,
            int? index = null,
            ILabel label = null)
        {
            Contract.Requires(vw != null);
            Contract.Requires(actionDependentFeatureSerializer != null);
            Contract.Requires(example != null);
            Contract.Requires(actionDependentFeatures != null);

            ActionDependentFeature<TActionDependentFeature>[] predictions = null;

            Execute(
                vw,
                serializer,
                actionDependentFeatureSerializer,
                example,
                actionDependentFeatures,
                (examples, validActionDependentFeatures, emptyActionDependentFeatures) =>
                {
                    var ex_col = examples.ToList();
                    vw.Predict(ex_col);
                    predictions = VowpalWabbitMultiLine.GetPrediction(vw, examples, validActionDependentFeatures, emptyActionDependentFeatures);
                },
                index,
                label);

            // default to the input list
            return predictions ?? actionDependentFeatures.Select((o, i) => new ActionDependentFeature<TActionDependentFeature>(i, o)).ToArray();
        }

        /// <summary>
        /// Extracts the prediction, orders the action depdendent feature objects accordingly and appends the
        /// action dependent feature objcts that did produce empty examples at the end.
        /// </summary>
        /// <typeparam name="TActionDependentFeature">The action dependent feature type.</typeparam>
        /// <param name="vw">The Vowpal Wabbit instance.></param>
        /// <param name="examples">The list of examples.</param>
        /// <param name="validActionDependentFeatures">The list of non-empty action dependent feature objects.</param>
        /// <param name="emptyActionDependentFeatures">The list of empty action dependent feature objects.</param>
        /// <returns>Returns the ranked list of action dependent features.</returns>
        public static ActionDependentFeature<TActionDependentFeature>[] GetPrediction<TActionDependentFeature>(
            VowpalWabbit vw,
            IReadOnlyList<VowpalWabbitExample> examples,
            IReadOnlyList<ActionDependentFeature<TActionDependentFeature>> validActionDependentFeatures,
            IReadOnlyList<ActionDependentFeature<TActionDependentFeature>> emptyActionDependentFeatures)
        {
            // Since the prediction result is stored in the first example
            // and we'll have to get an actual VowpalWabbitExampt
            var firstExample = examples.FirstOrDefault();
            if (firstExample == null)
            {
                return null;
            }

            ActionDependentFeature<TActionDependentFeature>[] result;
            int i = 0;

            var values = firstExample.GetPrediction(vw, VowpalWabbitPredictionType.Dynamic);
            var actionScores = values as ActionScore[];
            if (actionScores != null)
            {
                if (actionScores.Length != validActionDependentFeatures.Count)
                    throw new InvalidOperationException("Number of predictions returned unequal number of examples fed");

                result = new ActionDependentFeature<TActionDependentFeature>[validActionDependentFeatures.Count + emptyActionDependentFeatures.Count];

                foreach (var index in actionScores)
                {
                    result[i] = validActionDependentFeatures[(int)index.Action];
                    result[i].Probability = index.Score;
                    i++;
                }
            }
            else
            {
                var multilabel = values as int[];
                if (multilabel != null)
                {
                    if (multilabel.Length != validActionDependentFeatures.Count)
                        throw new InvalidOperationException("Number of predictions returned unequal number of examples fed");

                    result = new ActionDependentFeature<TActionDependentFeature>[validActionDependentFeatures.Count + emptyActionDependentFeatures.Count];

                    foreach (var index in multilabel)
                        result[i++] = validActionDependentFeatures[index];

                    result[0].Probability = 1f;
                }
                else
                    throw new NotSupportedException("Unsupported return type: " + values.GetType());
            }

            // append invalid ones at the end
            foreach (var f in emptyActionDependentFeatures)
                result[i++] = f;

            return result;
        }
    }
}

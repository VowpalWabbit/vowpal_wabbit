using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW.Interfaces;
using VW.Labels;
using VW.Serializer;
using VW.Serializer.Visitors;

namespace VW
{
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
        /// <param name="example">The user example.</param>
        /// <returns>An ordered subset of predicted action indexes.</returns>
        public static int[] LearnAndPredictIndex<TExample, TActionDependentFeature>(
            VowpalWabbit vw,
            VowpalWabbitSerializer<TExample> serializer,
            VowpalWabbitSerializer<TActionDependentFeature> actionDependentFeatureSerializer, 
            TExample example, IEnumerable<TActionDependentFeature> actionDependentFeatures, 
            int index, 
            ILabel label)
        {
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
        /// <param name="example">The user example.</param>
        /// <returns>An ordered subset of predicted action dependent features.</returns>
        public static TActionDependentFeature[] LearnAndPredict<TExample, TActionDependentFeature>(
            VowpalWabbit vw,
            VowpalWabbitSerializer<TExample> serializer,
            VowpalWabbitSerializer<TActionDependentFeature> actionDependentFeatureSerializer,
            TExample example, 
            IEnumerable<TActionDependentFeature> actionDependentFeatures, 
            int index, 
            ILabel label)
        {
            var multiLabelPredictions = LearnAndPredictIndex(vw, serializer, actionDependentFeatureSerializer, example, actionDependentFeatures, index, label);
            return actionDependentFeatures.Subset(multiLabelPredictions);
        }

        /// <summary>
        /// Simplify prediction of examples with action dependent features.
        /// </summary>
        /// <param name="example">The user example.</param>
        /// <returns>An ordered subset of predicted action indexes.</returns>
        public static int[] PredictIndex<TExample, TActionDependentFeature>(
            VowpalWabbit vw,
            VowpalWabbitSerializer<TExample> serializer,
            VowpalWabbitSerializer<TActionDependentFeature> actionDependentFeatureSerializer, 
            TExample example, 
            IEnumerable<TActionDependentFeature> actionDependentFeatures)
        {
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
        /// <param name="example">The user example.</param>
        /// <returns>An ordered subset of predicted action dependent features.</returns>
        public static TActionDependentFeature[] Predict<TExample, TActionDependentFeature>(
            VowpalWabbit vw,
            VowpalWabbitSerializer<TExample> serializer,
            VowpalWabbitSerializer<TActionDependentFeature> actionDependentFeatureSerializer, 
            TExample example,
            IEnumerable<TActionDependentFeature> actionDependentFeatures)
        {
            var multiLabelPredictions = PredictIndex(vw, serializer, actionDependentFeatureSerializer, example, actionDependentFeatures);
            return actionDependentFeatures.Subset(multiLabelPredictions);
        }
    }
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW.Interfaces;
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
            VowpalWabbitNative vw, 
            VowpalWabbitSerializer<TExample> serializer,
            VowpalWabbitSerializer<TActionDependentFeature> actionDependentFeatureSerializer,
            TExample example, 
            IEnumerable<TActionDependentFeature> actionDependentFeatures, 
            int index, 
            ILabel label)
        {
            var visitor = new VowpalWabbitInterfaceVisitor(vw);
            var examples = new List<IVowpalWabbitExample>();

            try
            {
                // contains prediction results
                var sharedExample = serializer.Serialize(visitor, example, SharedLabel.SharedLabel);
                // check if we have shared features
                if (sharedExample != null)
                {
                    examples.Add(sharedExample);
                    sharedExample.Learn();
                }

                var i = 0;
                foreach (var actionDependentFeature in actionDependentFeatures)
                {
                    var adfExample = actionDependentFeatureSerializer.Serialize(actionDependentFeature, i == index ? label : null);
                    examples.Add(adfExample);

                    adfExample.Learn();

                    i++;
                }

                // signal we're finished using an empty example
                // TODO: move empty example into VowpalWabbitNative
                emptyExample.Learn();

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
            VowpalWabbitNative vw,
            VowpalWabbitSerializer<TExample> serializer,
            VowpalWabbitSerializer<TActionDependentFeature> actionDependentFeatureSerializer, 
            TExample example, IEnumerable<TActionDependentFeature> actionDependentFeatures, 
            int index, 
            ILabel label)
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
                var i = 0;
                foreach (var actionDependentFeature in actionDependentFeatures)
                {
                    var adfExample = this.actionDependentFeatureSerializer.Serialize(actionDependentFeature, i == index ? label : null);
                    examples.Add(adfExample);

                    adfExample.Learn();
                    i++;
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

        /// <summary>
        /// Simplify learning of examples with action dependent features. 
        /// </summary>
        /// <param name="example">The user example.</param>
        /// <returns>An ordered subset of predicted action dependent features.</returns>
        public static TActionDependentFeature[] LearnAndPredict<TExample, TActionDependentFeature>(
            VowpalWabbitNative vw,
            VowpalWabbitSerializer<TExample> serializer,
            VowpalWabbitSerializer<TActionDependentFeature> actionDependentFeatureSerializer,
            TExample example, 
            IReadOnlyCollection<TActionDependentFeature> actionDependentFeatures, 
            int index, 
            ILabel label)
        {
            var multiLabelPredictions = this.LearnAndPredictIndex(example, actionDependentFeatures, index, label);
            return actionDependentFeatures.Permutate(multiLabelPredictions);
        }

        /// <summary>
        /// Simplify prediction of examples with action dependent features.
        /// </summary>
        /// <param name="example">The user example.</param>
        /// <returns>An ordered subset of predicted action indexes.</returns>
        public static int[] PredictIndex(
            VowpalWabbitNative vw,
            VowpalWabbitSerializer<TExample> serializer,
            VowpalWabbitSerializer<TActionDependentFeature> actionDependentFeatureSerializer, 
            TExample example, 
            IEnumerable<TActionDependentFeature> actionDependentFeatures)
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
                foreach (var actionDependentFeature in actionDependentFeatures)
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
        /// Simplify prediction of examples with action dependent features.
        /// </summary>
        /// <param name="example">The user example.</param>
        /// <returns>An ordered subset of predicted action dependent features.</returns>
        public static TActionDependentFeature[] Predict<TExample, TActionDependentFeature>(
            VowpalWabbitNative vw,
            VowpalWabbitSerializer<TExample> serializer,
            VowpalWabbitSerializer<TActionDependentFeature> actionDependentFeatureSerializer, 
            TExample example, 
            IReadOnlyCollection<TActionDependentFeature> actionDependentFeatures)
        {
            var multiLabelPredictions = this.PredictIndex(example, actionDependentFeatures);
            return actionDependentFeatures.Permutate(multiLabelPredictions);
        }
    }
}

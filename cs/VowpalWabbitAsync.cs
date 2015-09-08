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
    public class VowpalWabbitAsync<TExample> : IDisposable
    {
        private VowpalWabbitThreadedLearning manager;

        private VowpalWabbitSerializer<TExample>[] serializers;

        internal VowpalWabbitAsync(VowpalWabbitThreadedLearning manager)
        {
            // create a serializer for each instance - maintaining separate example caches
            this.serializers = Enumerable
                .Range(0, manager.Settings.ParallelOptions.MaxDegreeOfParallelism)
                .Select(_ => VowpalWabbitSerializerFactory.CreateSerializer<TExample>(manager.Settings))
                .ToArray();
        }

        public void Learn(TExample example, ILabel label)
        {
            manager.Post(vw =>
            {
                using (var ex = this.serializers[vw.Settings.Node].Serialize(vw, example, label))
                {
                    vw.Learn(ex);
                }
            });
        }

        public void Predict(TExample example)
        {
            manager.Post(vw =>
            {
                using (var ex = this.serializers[vw.Settings.Node].Serialize(vw, example))
                {
                    vw.Predict(ex);
                }
            });
        }

        public Task<TPrediction> Learn<TPrediction>(TExample example, ILabel label, IVowpalWabbitPredictionFactory<TPrediction> predictionFactory)
        {
            return manager.Post(vw =>
            {
                using (var ex = this.serializers[vw.Settings.Node].Serialize(vw, example, label))
                {
                    return vw.Learn(ex, predictionFactory);
                }
            });
        }

        public Task<TPrediction> Predict<TPrediction>(TExample example, IVowpalWabbitPredictionFactory<TPrediction> predictionFactory)
        {
            return manager.Post(vw =>
            {
                using (var ex = this.serializers[vw.Settings.Node].Serialize(vw, example))
                {
                    return vw.Predict(ex, predictionFactory);
                }
            });
        }

        /// <summary>
        /// Performs application-defined tasks associated with freeing, releasing, or resetting unmanaged resources.
        /// </summary>

        public void Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        private void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (this.serializers != null)
                {
                    foreach (var serializer in this.serializers)
                    {
                        // free cached examples
                        serializer.Dispose();
                    }

                    this.serializers = null;
                }
            }
        }
    }

    public class VowpalWabbitAsync<TExample, TActionDependentFeature> : IDisposable
    {
        private readonly VowpalWabbitThreadedLearning manager;

        private VowpalWabbitSerializer<TExample>[] serializers;

        private VowpalWabbitSerializer<TActionDependentFeature>[] actionDependentFeatureSerializers;

        internal VowpalWabbitAsync(VowpalWabbitThreadedLearning manager)
        {
            this.manager = manager;

            // create a serializer for each instance - maintaining separate example caches
            this.serializers = Enumerable
                .Range(0, manager.Settings.ParallelOptions.MaxDegreeOfParallelism)
                .Select(_ => VowpalWabbitSerializerFactory.CreateSerializer<TExample>(manager.Settings))
                .ToArray();

            this.actionDependentFeatureSerializers = Enumerable
                .Range(0, manager.Settings.ParallelOptions.MaxDegreeOfParallelism)
                .Select(_ => VowpalWabbitSerializerFactory.CreateSerializer<TActionDependentFeature>(manager.Settings))
                .ToArray();
        }

        public void Learn(TExample example, IEnumerable<TActionDependentFeature> actionDependentFeatures, int index, ILabel label)
        {
            manager.Post(vw => VowpalWabbitMultiLine.Learn(
                vw, 
                this.serializers[vw.Settings.Node], 
                this.actionDependentFeatureSerializers[vw.Settings.Node], 
                example, 
                actionDependentFeatures, 
                index, 
                label));
        }

        public Task<int[]> LearnAndPredictIndex(TExample example, IEnumerable<TActionDependentFeature> actionDependentFeatures, int index, ILabel label)
        {
            return manager.Post(vw => VowpalWabbitMultiLine.LearnAndPredictIndex(
                vw,
                this.serializers[vw.Settings.Node],
                this.actionDependentFeatureSerializers[vw.Settings.Node],
                example,
                actionDependentFeatures,
                index,
                label));
        }

        public Task<TActionDependentFeature[]> LearnAndPredict(TExample example, IEnumerable<TActionDependentFeature> actionDependentFeatures, int index, ILabel label)
        {
            return manager.Post(vw => VowpalWabbitMultiLine.LearnAndPredict(
                vw,
                this.serializers[vw.Settings.Node],
                this.actionDependentFeatureSerializers[vw.Settings.Node],
                example,
                actionDependentFeatures,
                index,
                label));
        }

        /// <summary>
        /// Performs application-defined tasks associated with freeing, releasing, or resetting unmanaged resources.
        /// </summary>
        public void Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        private void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (this.serializers != null)
                {
                    foreach (var serializer in this.serializers)
                    {
                        // free cached examples
                        serializer.Dispose();
                    }

                    this.serializers = null;
                }

                if (this.actionDependentFeatureSerializers != null)
                {
                    foreach (var serializer in this.actionDependentFeatureSerializers)
                    {
                        // free cached examples
                        serializer.Dispose();
                    }

                    this.actionDependentFeatureSerializers = null;
                }
            }
        }
    }
}

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
        private VowpalWabbitManager manager;

        private VowpalWabbitSerializer<TExample>[] serializers;

        internal VowpalWabbitAsync(VowpalWabbitManager manager)
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
                using (var nativeExample = this.serializers[vw.NodeId].Serialize(new VowpalWabbitInterfaceVisitor(vw), example, label))
                {
                    nativeExample.Learn();
                }
            });
        }

        public Task<TPrediction> LearnAndPredict<TPrediction>(TExample example, ILabel label)
            where TPrediction : class, VowpalWabbitPrediction, new()
        {
            return manager.Post(vw =>
            {
                using (var nativeExample = this.serializers[vw.NodeId].Serialize(new VowpalWabbitInterfaceVisitor(vw), example, label))
                {
                    return nativeExample.LearnAndPredict();
                }
            });
        }

        public Task<TPrediction> Predict<TPrediction>(TExample example)
            where TPrediction : class, VowpalWabbitPrediction, new()
        {
            return manager.Post(vw =>
            {
                using (var nativeExample = this.serializers[vw.NodeId].Serialize(new VowpalWabbitInterfaceVisitor(vw), example))
                {
                    return nativeExample.Predict();
                }
            });
        }

        public void PredictAndDiscard(TExample example)
        {
            manager.Post(vw =>
            {
                using (var nativeExample = this.serializers[vw.NodeId].Serialize(new VowpalWabbitInterfaceVisitor(vw), example))
                {
                    nativeExample.PredictAndDiscard();
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
            if (isDiposing)
            {
                if (this.serializers != null)
                {
                    foreach (var serializer in this.serializers)
                    {
                        // free cached examples
                        this.serializer.Dispose();
                    }

                    this.serializers = null;
                }
            }

            // don't dispose VW before we can dispose all cached examples
            // base.Dispose(isDiposing);
        }
    }

    public class VowpalWabbitAsync<TExample, TActionDependentFeature>
    {
        private VowpalWabbitManager manager;

        private VowpalWabbitSerializer<TExample>[] serializers;

        private VowpalWabbitSerializer<TActionDependentFeature>[] actionDependentFeatureSerializers;

        internal VowpalWabbitAsync(VowpalWabbitManager manager, VowpalWabbitThreaded[] vws)
        {
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
                this.serializers[vw.NodeId], 
                this.actionDependentFeatureSerializers[vw.NodeId], 
                example, 
                actionDependentFeatures, 
                index, 
                label));

            // build learn threading, predict threading, single instance
        }

        public Task<int[]> LearnAndPredictIndex(TExample example, IEnumerable<TActionDependentFeature> actionDependentFeatures, int index, ILabel label)
        {
            return manager.Post(vw => vws[vw.NodeId].LearnAndPredictIndex(example, actionDependentFeatures, index, label));
        }

        public TActionDependentFeature[] LearnAndPredict(TExample example, IReadOnlyCollection<TActionDependentFeature> actionDependentFeatures, int index, ILabel label)
        {
            return manager.Post(vw => vws[vw.NodeId].LearnAndPredict(example, actionDependentFeatures, index, label));
        }
    }

}

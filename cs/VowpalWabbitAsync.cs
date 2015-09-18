// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitAsync.cs">
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
using System.Threading.Tasks;
using VW.Interfaces;
using VW.Serializer;

namespace VW
{
    /// <summary>
    /// An async wrapper VW supporting data ingest using declarative serializer infrastructure used with <see cref="VowpalWabbitThreadedLearning"/>.
    /// </summary>
    /// <typeparam name="TExample">The user type to be serialized.</typeparam>
    public class VowpalWabbitAsync<TExample> : IDisposable
    {
        /// <summary>
        /// The owning manager.
        /// </summary>
        private VowpalWabbitThreadedLearning manager;

        /// <summary>
        /// The serializers are not thread-safe. Thus we need to allocate one for each VW instance.
        /// </summary>
        private VowpalWabbitSerializer<TExample>[] serializers;

        internal VowpalWabbitAsync(VowpalWabbitThreadedLearning manager)
        {
            Contract.Requires(manager != null);
            Contract.Ensures(this.serializers != null);

            this.manager = manager;

            // create a serializer for each instance - maintaining separate example caches
            // TODO
            //this.serializers = Enumerable
            //    .Range(0, manager.Settings.ParallelOptions.MaxDegreeOfParallelism)
            //    .Select(_ => VowpalWabbitSerializerFactory.CreateSerializer<TExample>())
            //    .ToArray();
        }

        /// <summary>
        /// Learns from the given example.
        /// </summary>
        /// <param name="example">The example to learn.</param>
        /// <param name="label">The label for this <paramref name="example"/>.</param>
        /// <remarks>
        /// The method only enqueues the example for learning and returns immediately.
        /// You must not re-use the example.
        /// </remarks>
        public void Learn(TExample example, ILabel label)
        {
            Contract.Requires(example != null);
            Contract.Requires(label != null);

            manager.Post(vw =>
            {
                using (var ex = this.serializers[vw.Settings.Node].Serialize(example, label))
                {
                    vw.Learn(ex);
                }
            });
        }

        /// <summary>
        /// Predicts for the given example.
        /// </summary>
        /// <param name="example">The example to predict for.</param>
        /// <remarks>
        /// The method only enqueues the example for prediction and returns immediately.
        /// You must not re-use the example.
        /// </remarks>
        public void Predict(TExample example)
        {
            Contract.Requires(example != null);

            manager.Post(vw =>
            {
                using (var ex = this.serializers[vw.Settings.Node].Serialize(example))
                {
                    vw.Predict(ex);
                }
            });
        }

        /// <summary>
        /// Learns from the given example.
        /// </summary>
        /// <param name="example">The example to learn.</param>
        /// <param name="label">The label for this <paramref name="example"/>.</param>
        /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
        /// <returns>The prediction for the given <paramref name="example"/>.</returns>
        /// <remarks>
        /// The method only enqueues the example for learning and returns immediately.
        /// Await the returned task to receive the prediction result.
        /// </remarks>
        public Task<TPrediction> Learn<TPrediction>(TExample example, ILabel label, IVowpalWabbitPredictionFactory<TPrediction> predictionFactory)
        {
            Contract.Requires(example != null);
            Contract.Requires(label != null);
            Contract.Requires(predictionFactory != null);

            return manager.Post(vw =>
            {
                using (var ex = this.serializers[vw.Settings.Node].Serialize(example, label))
                {
                    return vw.Learn(ex, predictionFactory);
                }
            });
        }

        /// <summary>
        /// Predicts for the given example.
        /// </summary>
        /// <param name="example">The example to predict for.</param>
        /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
        /// <returns>The prediction for the given <paramref name="example"/>.</returns>
        /// <remarks>
        /// The method only enqueues the example for learning and returns immediately.
        /// Await the returned task to receive the prediction result.
        /// </remarks>
        public Task<TPrediction> Predict<TPrediction>(TExample example, IVowpalWabbitPredictionFactory<TPrediction> predictionFactory)
        {
            Contract.Requires(example != null);
            Contract.Requires(predictionFactory != null);

            return manager.Post(vw =>
            {
                using (var ex = this.serializers[vw.Settings.Node].Serialize(example))
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

    /// <summary>
    /// An async VW wrapper for multiline ingest.
    /// </summary>
    /// <typeparam name="TExample">The user type of the shared feature.</typeparam>
    /// <typeparam name="TActionDependentFeature">The user type for each action dependent feature.</typeparam>
    public class VowpalWabbitAsync<TExample, TActionDependentFeature> : IDisposable
    {
        /// <summary>
        /// The owning manager.
        /// </summary>
        private readonly VowpalWabbitThreadedLearning manager;

        /// <summary>
        /// The serializers are not thread-safe. Thus we need to allocate one for each VW instance.
        /// </summary>
        private VowpalWabbitSerializer<TExample>[] serializers;

        /// <summary>
        /// The serializers are not thread-safe. Thus we need to allocate one for each VW instance.
        /// </summary>
        private VowpalWabbitSerializer<TActionDependentFeature>[] actionDependentFeatureSerializers;

        internal VowpalWabbitAsync(VowpalWabbitThreadedLearning manager)
        {
            if (manager == null)
            {
                throw new ArgumentNullException("manager");
            }

            if (manager.Settings == null)
            {
                throw new ArgumentNullException("manager.Settings");
            }

            if (manager.Settings.ParallelOptions == null)
            {
                throw new ArgumentNullException("manager.Settings.ParallelOptions");
            }

            if (manager.Settings.ParallelOptions.MaxDegreeOfParallelism <= 0)
            {
                throw new ArgumentOutOfRangeException("MaxDegreeOfParallelism must be greater than zero.");
            }

            Contract.Ensures(this.serializers != null);
            Contract.Ensures(this.actionDependentFeatureSerializers != null);
            Contract.EndContractBlock();

            this.manager = manager;

            // create a serializer for each instance - maintaining separate example caches
            // TODO:
            //this.serializers = Enumerable
            //    .Range(0, manager.Settings.ParallelOptions.MaxDegreeOfParallelism)
            //    .Select(_ => VowpalWabbitSerializerFactory.CreateSerializer<TExample>(manager.Settings))
            //    .ToArray();

            //this.actionDependentFeatureSerializers = Enumerable
            //    .Range(0, manager.Settings.ParallelOptions.MaxDegreeOfParallelism)
            //    .Select(_ => VowpalWabbitSerializerFactory.CreateSerializer<TActionDependentFeature>(manager.Settings))
            //    .ToArray();
        }

        /// <summary>
        /// Learn from the given example and return the current prediction for it.
        /// </summary>
        /// <param name="example">The shared example.</param>
        /// <param name="actionDependentFeatures">The action dependent features.</param>
        /// <param name="index">The index of the example to learn within <paramref name="actionDependentFeatures"/>.</param>
        /// <param name="label">The label for the example to learn.</param>
        public void Learn(TExample example, IEnumerable<TActionDependentFeature> actionDependentFeatures, int index, ILabel label)
        {
            Contract.Requires(example != null);
            Contract.Requires(actionDependentFeatures != null);
            Contract.Requires(index >= 0);
            Contract.Requires(label != null);

            manager.Post(vw => VowpalWabbitMultiLine.Learn(
                vw,
                this.serializers[vw.Settings.Node],
                this.actionDependentFeatureSerializers[vw.Settings.Node],
                example,
                actionDependentFeatures,
                index,
                label));
        }

        /// <summary>
        /// Learn from the given example and return the current prediction for it.
        /// </summary>
        /// <param name="example">The shared example.</param>
        /// <param name="actionDependentFeatures">The action dependent features.</param>
        /// <param name="index">The index of the example to learn within <paramref name="actionDependentFeatures"/>.</param>
        /// <param name="label">The label for the example to learn.</param>
        /// <returns>The ranked prediction for the given examples.</returns>
        public Task<int[]> LearnAndPredictIndex(TExample example, IEnumerable<TActionDependentFeature> actionDependentFeatures, int index, ILabel label)
        {
            Contract.Requires(example != null);
            Contract.Requires(actionDependentFeatures != null);
            Contract.Requires(index >= 0);
            Contract.Requires(label != null);
            Contract.Ensures(Contract.Result<Task<int[]>>() != null);

            return manager.Post(vw => VowpalWabbitMultiLine.LearnAndPredictIndex(
                vw,
                this.serializers[vw.Settings.Node],
                this.actionDependentFeatureSerializers[vw.Settings.Node],
                example,
                actionDependentFeatures,
                index,
                label));
        }

        /// <summary>
        /// Learn from the given example and return the current prediction for it.
        /// </summary>
        /// <param name="example">The shared example.</param>
        /// <param name="actionDependentFeatures">The action dependent features.</param>
        /// <param name="index">The index of the example to learn within <paramref name="actionDependentFeatures"/>.</param>
        /// <param name="label">The label for the example to learn.</param>
        /// <returns>The ranked prediction for the given examples.</returns>
        public Task<TActionDependentFeature[]> LearnAndPredict(TExample example, IEnumerable<TActionDependentFeature> actionDependentFeatures, int index, ILabel label)
        {
            Contract.Requires(example != null);
            Contract.Requires(actionDependentFeatures != null);
            Contract.Requires(index >= 0);
            Contract.Requires(label != null);
            Contract.Ensures(Contract.Result<Task<TActionDependentFeature[]>>() != null);

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

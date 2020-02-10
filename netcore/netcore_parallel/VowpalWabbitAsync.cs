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
using VW.Labels;
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
        private IVowpalWabbitSerializer<TExample>[] serializers;

        internal VowpalWabbitAsync(VowpalWabbitThreadedLearning manager)
        {
            Contract.Requires(manager != null);
            Contract.Ensures(this.serializers != null);

            this.manager = manager;

            // create a serializer for each instance - maintaining separate example caches
            var serializer = VowpalWabbitSerializerFactory.CreateSerializer<TExample>(manager.Settings);
            this.serializers = this.manager.VowpalWabbits
                .Select(vw => serializer.Create(vw))
                .ToArray();
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
        public void Learn(TExample example, ILabel label = null)
        {
            Contract.Requires(example != null);
            Contract.Requires(label != null);

            manager.Post(vw =>
            {
                using (var ex = this.serializers[vw.Settings.Node].Serialize(example, label))
                {
                    ex.Learn();
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
                    ex.Predict();
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
                    return ex.Learn(predictionFactory);
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
                    return ex.Predict(predictionFactory);
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
        private VowpalWabbitSingleExampleSerializer<TExample>[] serializers;

        /// <summary>
        /// The serializers are not thread-safe. Thus we need to allocate one for each VW instance.
        /// </summary>
        private VowpalWabbitSingleExampleSerializer<TActionDependentFeature>[] actionDependentFeatureSerializers;

        internal VowpalWabbitAsync(VowpalWabbitThreadedLearning manager)
        {
            if (manager == null)
                throw new ArgumentNullException("manager");

            if (manager.Settings == null)
                throw new ArgumentNullException("manager.Settings");

            if (manager.Settings.ParallelOptions == null)
                throw new ArgumentNullException("manager.Settings.ParallelOptions");

            if (manager.Settings.ParallelOptions.MaxDegreeOfParallelism <= 0)
                throw new ArgumentOutOfRangeException("MaxDegreeOfParallelism must be greater than zero.");

            Contract.Ensures(this.serializers != null);
            Contract.Ensures(this.actionDependentFeatureSerializers != null);
            Contract.EndContractBlock();

            this.manager = manager;

            // create a serializer for each instance - maintaining separate example caches
            var serializer = VowpalWabbitSerializerFactory.CreateSerializer<TExample>(manager.Settings) as VowpalWabbitSingleExampleSerializerCompiler<TExample>;
            if (serializer == null)
                throw new ArgumentException(string.Format(
                "{0} maps to a multiline example. Use VowpalWabbitAsync<{0}> instead.",
                    typeof(TExample)));

            var adfSerializer = VowpalWabbitSerializerFactory.CreateSerializer<TActionDependentFeature>(manager.Settings) as VowpalWabbitSingleExampleSerializerCompiler<TActionDependentFeature>;
            if (adfSerializer == null)
                throw new ArgumentException(string.Format(
                "{0} maps to a multiline example. Use VowpalWabbitAsync<{0}> instead.",
                    typeof(TActionDependentFeature)));

            this.serializers = this.manager.VowpalWabbits
                .Select(vw => serializer.Create(vw))
                .ToArray();

            this.actionDependentFeatureSerializers = this.manager.VowpalWabbits
                .Select(vw => adfSerializer.Create(vw))
                .ToArray();
        }

        /// <summary>
        /// Learn from the given example and return the current prediction for it.
        /// </summary>
        /// <param name="example">The shared example.</param>
        /// <param name="actionDependentFeatures">The action dependent features.</param>
        /// <param name="index">The index of the example to learn within <paramref name="actionDependentFeatures"/>.</param>
        /// <param name="label">The label for the example to learn.</param>
        public void Learn(TExample example, IReadOnlyCollection<TActionDependentFeature> actionDependentFeatures, int index, ILabel label)
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
        public Task<ActionDependentFeature<TActionDependentFeature>[]> LearnAndPredict(TExample example, IReadOnlyCollection<TActionDependentFeature> actionDependentFeatures, int index, ILabel label)
        {
            Contract.Requires(example != null);
            Contract.Requires(actionDependentFeatures != null);
            Contract.Requires(index >= 0);
            Contract.Requires(label != null);

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

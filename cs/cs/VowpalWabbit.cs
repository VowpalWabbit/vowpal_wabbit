// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbit.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using VW.Labels;
using VW.Serializer;

namespace VW
{
    /// <summary>
    /// VW wrapper supporting data ingest using declarative serializer infrastructure.
    /// </summary>
    /// <typeparam name="TExample">The user type to be serialized.</typeparam>
    public class VowpalWabbit<TExample> : IDisposable
    {
        /// <summary>
        /// Native vw instance.
        /// </summary>
        private VowpalWabbit vw;

        /// <summary>
        /// The example serializer.
        /// </summary>
        private IVowpalWabbitSerializer<TExample> serializer;

        /// <summary>
        /// The example serializer compilation. Useful when debugging.
        /// </summary>
        private IVowpalWabbitSerializerCompiler<TExample> compiledSerializer;

        /// <summary>
        /// The serializer used for learning. It's only set if the serializer is non-caching.
        /// By having a second field there is one less check that has to be done in the hot path.
        /// </summary>
        private readonly IVowpalWabbitSerializer<TExample> learnSerializer;

        private readonly VowpalWabbitSingleExampleSerializer<TExample> singleLineSerializer;

        /// <summary>
        /// Initializes a new instance of the <see cref="VowpalWabbit{TExample}"/> class.
        /// </summary>
        /// <param name="args">Command line arguments passed to native instance.</param>
        public VowpalWabbit(String args) : this(new VowpalWabbit(args))
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="VowpalWabbit{TExample}"/> class.
        /// </summary>
        /// <param name="settings">Arguments passed to native instance.</param>
        public VowpalWabbit(VowpalWabbitSettings settings)
            : this(new VowpalWabbit(settings))
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="VowpalWabbit{TExample}"/> class.
        /// </summary>
        /// <param name="vw">The native instance to wrap.</param>
        /// <remarks>This instance takes ownership of <paramref name="vw"/> instance and disposes it.</remarks>
        public VowpalWabbit(VowpalWabbit vw) : this(vw, VowpalWabbitSerializerFactory.CreateSerializer<TExample>(vw.Settings))
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="VowpalWabbit{TExample}"/> class.
        /// </summary>
        /// <param name="vw">The native instance to wrap.</param>
        /// <param name="compiledSerializer">The per-compiled serializer.</param>
        /// <remarks>This instance takes ownership of <paramref name="vw"/> instance and disposes it.</remarks>
        public VowpalWabbit(VowpalWabbit vw, IVowpalWabbitSerializerCompiler<TExample> compiledSerializer)
        {
            if (vw == null)
                throw new ArgumentNullException(nameof(vw));
            if (compiledSerializer == null)
                throw new ArgumentNullException(nameof(compiledSerializer));
            Contract.Ensures(this.serializer != null);
            Contract.EndContractBlock();

            this.vw = vw;
            this.compiledSerializer = compiledSerializer;

            this.serializer = this.compiledSerializer.Create(vw);

            // have a 2nd member to throw NullReferenceException in release instead of silently producing wrong results.
            this.learnSerializer = this.serializer.CachesExamples ? null : this.serializer;

            // have a 3rd member to avoid cast everytime...
            this.singleLineSerializer = this.serializer as VowpalWabbitSingleExampleSerializer<TExample>;
        }

        /// <summary>
        /// The wrapped VW instance.
        /// </summary>
        public VowpalWabbit Native
        {
            get
            {
                return this.vw;
            }
        }

        /// <summary>
        /// The serializer used to marshal examples.
        /// </summary>
        public IVowpalWabbitSerializerCompiler<TExample> Serializer
        {
            get
            {
                return this.compiledSerializer;
            }
        }

        /// <summary>
        /// Learns from the given example.
        /// </summary>
        /// <param name="example">The example to learn.</param>
        /// <param name="label">The label for this <paramref name="example"/>.</param>
        /// <param name="index">The optional index of the example, the <paramref name="label"/> should be attributed to.</param>
        public void Learn(TExample example, ILabel label = null, int? index = null)
        {
            Contract.Requires(example != null);
            Contract.Requires(label != null);

#if DEBUG
            if (this.serializer.CachesExamples)
            {
                throw new NotSupportedException("Cached examples cannot be used for learning");
            }
#endif

            // in release this throws NullReferenceException instead of producing silently wrong results
            using (var ex = this.learnSerializer.Serialize(example, label, index))
            {
                ex.Learn();
            }
        }

        /// <summary>
        /// Learn from the given example and returns the current prediction for it.
        /// </summary>
        /// <typeparam name="TPrediction">The prediction type.</typeparam>
        /// <param name="example">The example to learn.</param>
        /// <param name="label">The label for this <paramref name="example"/>.</param>
        /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
        /// <returns>The prediction for the given <paramref name="example"/>.</returns>
        public TPrediction Learn<TPrediction>(TExample example, ILabel label, IVowpalWabbitPredictionFactory<TPrediction> predictionFactory)
        {
            Contract.Requires(example != null);
            Contract.Requires(label != null);
            Contract.Requires(predictionFactory != null);

#if DEBUG
            // only in debug, since it's a hot path
            if (this.serializer.CachesExamples)
            {
                throw new NotSupportedException("Cached examples cannot be used for learning");
            }
#endif

            using (var ex = this.learnSerializer.Serialize(example, label))
            {
                return ex.Learn(predictionFactory);
            }
        }

        /// <summary>
        /// Predicts for the given example.
        /// </summary>
        /// <param name="example">The example to predict for.</param>
        /// <param name="label">This label can be used to weight the example.</param>
        public void Predict(TExample example, ILabel label = null)
        {
            Contract.Requires(example != null);

            using (var ex = this.serializer.Serialize(example, label))
            {
                ex.Predict();
            }
        }

        /// <summary>
        /// Predicts for the given example.
        /// </summary>
        /// <typeparam name="TPrediction">The prediction type.</typeparam>
        /// <param name="example">The example to predict for.</param>
        /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
        /// <param name="label">This label can be used to weight the example.</param>
        public TPrediction Predict<TPrediction>(TExample example, IVowpalWabbitPredictionFactory<TPrediction> predictionFactory, ILabel label = null)
        {
            Contract.Requires(example != null);
            Contract.Requires(predictionFactory != null);

            using (var ex = this.serializer.Serialize(example, label))
            {
                return ex.Predict(predictionFactory);
            }
        }

        /// <summary>
        /// Learn from the given example and return the current prediction for it.
        /// </summary>
        /// <param name="actionDependentFeatures">The action dependent features.</param>
        /// <param name="index">The index of the example to learn within <paramref name="actionDependentFeatures"/>.</param>
        /// <param name="label">The label for the example to learn.</param>
        public void Learn(IReadOnlyCollection<TExample> actionDependentFeatures, int index, ILabel label)
        {
            Contract.Requires(actionDependentFeatures != null);
            Contract.Requires(this.singleLineSerializer != null,
                string.Format(
                "{0} maps to a multiline example. Use VowpalWabbit.Learn<{0}>({0} example,...) instead.",
                    typeof(TExample)));

            VowpalWabbitMultiLine.Learn<object, TExample>(
                this.vw,
                null,
                this.singleLineSerializer,
                null,
                actionDependentFeatures,
                index,
                label);
        }

        /// <summary>
        /// Learn from the given example and return the current prediction for it.
        /// </summary>
        /// <param name="actionDependentFeatures">The action dependent features.</param>
        /// <param name="index">The index of the example to learn within <paramref name="actionDependentFeatures"/>.</param>
        /// <param name="label">The label for the example to learn.</param>
        /// <returns>The ranked prediction for the given examples.</returns>
        public ActionDependentFeature<TExample>[] LearnAndPredict(IReadOnlyCollection<TExample> actionDependentFeatures, int index, ILabel label)
        {
            Contract.Requires(actionDependentFeatures != null);
            Contract.Requires(index >= 0);
            Contract.Requires(label != null);
            Contract.Requires(this.singleLineSerializer != null,
                string.Format(
                "{0} maps to a multiline example. Use VowpalWabbit.Learn<{0}>({0} example,...) instead.",
                    typeof(TExample)));

            return VowpalWabbitMultiLine.LearnAndPredict<object, TExample>(
                this.vw,
                null,
                this.singleLineSerializer,
                null,
                actionDependentFeatures,
                index,
                label);
        }

        /// <summary>
        /// Predict for the given example and return the current prediction for it.
        /// </summary>
        /// <param name="actionDependentFeatures">The action dependent features.</param>
        /// <param name="index">The index of the example to evaluate within <paramref name="actionDependentFeatures"/>.</param>
        /// <param name="label">The label for the example to evaluate.</param>
        /// <returns>The ranked prediction for the given examples.</returns>
        public ActionDependentFeature<TExample>[] Predict(IReadOnlyCollection<TExample> actionDependentFeatures, int? index = null, ILabel label = null)
        {
            Contract.Requires(actionDependentFeatures != null);
            Contract.Requires(this.singleLineSerializer != null,
                string.Format(
                "{0} maps to a multiline example. Use VowpalWabbit.Learn<{0}>({0} example,...) instead.",
                    typeof(TExample)));

            return VowpalWabbitMultiLine.Predict<object, TExample>(
                this.vw,
                null,
                this.singleLineSerializer,
                null,
                actionDependentFeatures,
                index,
                label);
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
                if (this.serializer != null)
                {
                    this.serializer.Dispose();
                    this.serializer = null;
                }

                if (this.vw != null)
                {
                    this.vw.Dispose();
                    this.vw = null;
                }
            }
        }
    }

    /// <summary>
    /// VW wrapper for multiline ingest.
    /// </summary>
    /// <typeparam name="TExample">The user type of the shared feature.</typeparam>
    /// <typeparam name="TActionDependentFeature">The user type for each action dependent feature.</typeparam>
    public class VowpalWabbit<TExample, TActionDependentFeature> : IDisposable
    {
        /// <summary>
        /// The native wrapper.
        /// </summary>
        private VowpalWabbit vw;

        /// <summary>
        /// The shared example serializer.
        /// </summary>
        private VowpalWabbitSingleExampleSerializer<TExample> serializer;

        /// <summary>
        /// The action dependent feature serializer.
        /// </summary>
        private VowpalWabbitSingleExampleSerializer<TActionDependentFeature> actionDependentFeatureSerializer;

        /// <summary>
        /// The action dependent feature serializer valid for learning. If example caching is enabled, this is null.
        /// </summary>
        private readonly VowpalWabbitSingleExampleSerializer<TActionDependentFeature> actionDependentFeatureLearnSerializer;

        /// <summary>
        /// Initializes a new instance of the <see cref="VowpalWabbit{TExample,TActionDependentFeature}"/> class.
        /// </summary>
        /// <param name="args">Command line arguments passed to native instance.</param>
        public VowpalWabbit(String args)
            : this(new VowpalWabbit(args))
        { }

        /// <summary>
        /// Initializes a new instance of the <see cref="VowpalWabbit{TExample,TActionDependentFeature}"/> class.
        /// </summary>
        /// <param name="settings">Arguments passed to native instance.</param>
        public VowpalWabbit(VowpalWabbitSettings settings)
            : this(new VowpalWabbit(settings))
        { }

        /// <summary>
        /// Initializes a new instance of the <see cref="VowpalWabbit{TExample,TActionDependentFeature}"/> class.
        /// </summary>
        /// <param name="vw">The native instance to wrap.</param>
        /// <remarks>This instance takes ownership of <paramref name="vw"/> instance and disposes it.</remarks>
        public VowpalWabbit(VowpalWabbit vw)
        {
            if (vw == null)
            {
                throw new ArgumentNullException("vw");
            }
            Contract.EndContractBlock();

            this.vw = vw;
            this.serializer = VowpalWabbitSerializerFactory.CreateSerializer<TExample>(vw.Settings).Create(vw) as VowpalWabbitSingleExampleSerializer<TExample>;
            if (this.serializer == null)
                throw new ArgumentException(string.Format(
                    "{0} maps to a multiline example. Use VowpalWabbit<{0}> instead.",
                        typeof(TExample)));

            var adfSettings = (VowpalWabbitSettings)vw.Settings.Clone();
            adfSettings.Schema = vw.Settings.ActionDependentSchema;
            this.actionDependentFeatureSerializer = VowpalWabbitSerializerFactory.CreateSerializer<TActionDependentFeature>(adfSettings).Create(vw) as VowpalWabbitSingleExampleSerializer<TActionDependentFeature>;
            if (this.actionDependentFeatureSerializer == null)
                throw new ArgumentException(string.Format(
                    "{0} maps to a multiline example. Use VowpalWabbit<{0}> instead.",
                        typeof(TActionDependentFeature)));

            // have a 2nd member to throw NullReferenceException in release instead of silently producing wrong results.
            this.actionDependentFeatureLearnSerializer = this.actionDependentFeatureSerializer.CachesExamples ? null : this.actionDependentFeatureSerializer;
        }

        /// <summary>
        /// The wrapped VW instance.
        /// </summary>
        public VowpalWabbit Native
        {
            get
            {
                return this.vw;
            }
        }

        /// <summary>
        /// Internal example serializer.
        /// </summary>
        internal IVowpalWabbitSerializer<TExample> ExampleSerializer
        {
            get
            {
                return this.serializer;
            }
        }

        /// <summary>
        /// Internal action dependent feature serializer.
        /// </summary>
        internal IVowpalWabbitSerializer<TActionDependentFeature> ActionDependentFeatureSerializer
        {
            get
            {
                return this.actionDependentFeatureSerializer;
            }
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

            VowpalWabbitMultiLine.Learn(
                this.vw,
                this.serializer,
                this.actionDependentFeatureLearnSerializer,
                example,
                actionDependentFeatures,
                index,
                label);
        }

        /// <summary>
        /// Learn from the given example and return the current prediction for it.
        /// </summary>
        /// <param name="example">The shared example.</param>
        /// <param name="actionDependentFeatures">The action dependent features.</param>
        /// <param name="index">The index of the example to learn within <paramref name="actionDependentFeatures"/>.</param>
        /// <param name="label">The label for the example to learn.</param>
        /// <returns>The ranked prediction for the given examples.</returns>
        public ActionDependentFeature<TActionDependentFeature>[] LearnAndPredict(TExample example, IReadOnlyCollection<TActionDependentFeature> actionDependentFeatures, int index, ILabel label)
        {
            Contract.Requires(example != null);
            Contract.Requires(actionDependentFeatures != null);
            Contract.Requires(index >= 0);
            Contract.Requires(label != null);

            return VowpalWabbitMultiLine.LearnAndPredict(
                this.vw,
                this.serializer,
                this.actionDependentFeatureLearnSerializer,
                example,
                actionDependentFeatures,
                index,
                label);
        }

        /// <summary>
        /// Learn from the given example and return the current prediction for it.
        /// </summary>
        /// <param name="example">The shared example.</param>
        /// <param name="actionDependentFeatures">The action dependent features.</param>
        /// <param name="index">The index of the example to evaluate within <paramref name="actionDependentFeatures"/>.</param>
        /// <param name="label">The label for the example to evaluate.</param>
        /// <returns>The ranked prediction for the given examples.</returns>
        public ActionDependentFeature<TActionDependentFeature>[] Predict(TExample example, IReadOnlyCollection<TActionDependentFeature> actionDependentFeatures, int? index = null, ILabel label = null)
        {
            Contract.Requires(example != null);
            Contract.Requires(actionDependentFeatures != null);

            return VowpalWabbitMultiLine.Predict(
                this.vw,
                this.serializer,
                this.actionDependentFeatureSerializer,
                example,
                actionDependentFeatures,
                index,
                label);
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
                if (this.vw != null)
                {
                    this.vw.Dispose();
                    this.vw = null;
                }

                if (this.serializer != null)
                {
                    this.serializer.Dispose();
                    this.serializer = null;
                }

                if (this.actionDependentFeatureSerializer != null)
                {
                    this.actionDependentFeatureSerializer.Dispose();
                    this.actionDependentFeatureSerializer = null;
                }
            }
        }
    }
}

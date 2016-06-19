// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitDynamic.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using VW.Labels;
using VW.Serializer;

namespace VW
{
    /// <summary>
    /// Vowpal Wabbit wrapper for anonymous classes. Type used for serialization doesn't need to be known at compile time,
    /// but it's checked at runtime.
    /// </summary>
    /// <remarks>For each call to <see cref="Learn"/> there is additional overhead as the type is looked up in a dictionary compared to <see cref="VowpalWabbit{T}"/>.</remarks>
    public class VowpalWabbitDynamic : IDisposable
    {
        private Dictionary<Type, IDisposable> serializers;
        private Dictionary<Type, MethodInfo> serializeMethods;

        private VowpalWabbit vw;

        /// <summary>
        /// Initializes a new instance of the <see cref="VowpalWabbitDynamic"/> class.
        /// </summary>
        /// <param name="arguments">Command line arguments passed to native instance.</param>
        public VowpalWabbitDynamic(string arguments) : this(new VowpalWabbitSettings(arguments))
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="VowpalWabbitDynamic"/> class.
        /// </summary>
        /// <param name="settings">Arguments passed to native instance.</param>
        public VowpalWabbitDynamic(VowpalWabbitSettings settings)
        {
            this.vw = new VowpalWabbit(settings);
            this.serializers = new Dictionary<Type, IDisposable>();
            this.serializeMethods = new Dictionary<Type, MethodInfo>();
        }

        private VowpalWabbitExampleCollection SerializeTyped<T>(T example, ILabel label, int? index)
        {
            IDisposable serializer;
            if (!this.serializers.TryGetValue(typeof(T), out serializer))
            {
                var serializerCompiler = VowpalWabbitSerializerFactory.CreateSerializer<T>(this.vw.Settings);

                if (serializerCompiler == null)
                    throw new ArgumentException("No feature discovered for type: " + typeof(T));

                serializer = serializerCompiler.Create(this.vw);

                this.serializers.Add(typeof(T), serializer);
            }

            return ((IVowpalWabbitSerializer<T>)serializer).Serialize(example, label, index);
        }

        private VowpalWabbitExampleCollection Serialize(object example, ILabel label = null, int? index = null)
        {
            var type = example.GetType();
            MethodInfo method;
            if (!this.serializeMethods.TryGetValue(type, out method))
            {
                method = typeof(VowpalWabbitDynamic)
                    .GetMethod("SerializeTyped", BindingFlags.Instance | BindingFlags.NonPublic)
                    .MakeGenericMethod(type);

                this.serializeMethods.Add(type, method);
            }

            return (VowpalWabbitExampleCollection)method.Invoke(this, new[] { example, label, index });
        }

        /// <summary>
        /// Learns from the given example.
        /// </summary>
        /// <param name="example">The example to learn.</param>
        /// <param name="label">The optional label for this <paramref name="example"/>.</param>
        /// <param name="index">The optional index of the example, the <paramref name="label"/> should be attributed to.</param>
        public void Learn(object example, ILabel label = null, int? index = null)
        {
            Contract.Requires(example != null);

            using (var ex = this.Serialize(example, label, index))
            {
                ex.Learn();
            }
        }

        /// <summary>
        /// Learns from the given example and returns the current prediction.
        /// </summary>
        /// <param name="example">The example to learn.</param>
        /// <param name="predictionFactory">The prediction factory used to extract the prediction. Use <see cref="VowpalWabbitPredictionType"/>.</param>
        /// <param name="label">The optional label for this <paramref name="example"/>.</param>
        /// <param name="index">The optional index of the example, the <paramref name="label"/> should be attributed to.</param>
        public TPrediction Learn<TPrediction>(object example, IVowpalWabbitPredictionFactory<TPrediction> predictionFactory, ILabel label = null, int? index = null)
        {
            Contract.Requires(example != null);
            Contract.Requires(predictionFactory != null);

            using (var ex = this.Serialize(example, label, index))
            {
                return ex.Learn(predictionFactory);
            }
        }

        /// <summary>
        /// Predict for the given example and return the current prediction for it.
        /// </summary>
        /// <typeparam name="TPrediction"></typeparam>
        /// <param name="example"></param>
        /// <param name="predictionFactory"></param>
        /// <param name="index">The optional index of the example to evaluate within </param>
        /// <param name="label">The optional label for the example to evaluate.</param>
        /// <returns></returns>
        public TPrediction Predict<TPrediction>(object example, IVowpalWabbitPredictionFactory<TPrediction> predictionFactory, ILabel label = null, int? index = null)
        {
            Contract.Requires(example != null);
            Contract.Requires(predictionFactory != null);

            using (var ex = this.Serialize(example, label, index))
            {
                return ex.Predict(predictionFactory);
            }
        }

        /// <summary>
        /// The wrapped VW instance.
        /// </summary>
        public VowpalWabbit Native { get { return this.vw; } }

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
                        serializer.Value.Dispose();
                    
                    this.serializers = null;
                }

                if (this.vw != null)
                {
                    this.vw.Dispose();
                    this.vw = null;
                }
            }
        }

    }
}

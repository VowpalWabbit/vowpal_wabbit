// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitDynamic.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Text;
using System.Threading.Tasks;
using VW.Interfaces;
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
        private Dictionary<Type, IVowpalWabbitSerializer<object>> serializers;

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
        }

        private IVowpalWabbitSerializer<object> GetOrCreateSerializer(Type type)
        {
            IVowpalWabbitSerializer<object> serializer;
            if (!this.serializers.TryGetValue(type, out serializer))
            {
                var allFeatures = AnnotationInspector.ExtractFeatures(type, (_,__) => true);
                foreach (var feature in allFeatures)
                {
                    // inject type cast to the actual type (always works)
                    // needed since the serializer is generated for "type", not for "object"
                    feature.ValueExpressionFactory = expr => feature.ValueExpressionFactory(Expression.Convert(expr, type));
                }

                serializer = VowpalWabbitSerializerFactory
                    .CreateSerializer<object>(this.vw.Settings.ShallowCopy(allFeatures: allFeatures))
                    .Create(this.vw);

                this.serializers.Add(type, serializer);
            }

            return serializer;
        }

        /// <summary>
        /// Learns from the given example.
        /// </summary>
        /// <param name="example">The example to learn.</param>
        /// <param name="label">The label for this <paramref name="example"/>.</param>
        /// <param name="index">The optional index of the example, the <paramref name="label"/> should be attributed to.</param>
        public void Learn(object example, ILabel label, int? index = null)
        {
            using (var ex = GetOrCreateSerializer(example.GetType()).Serialize(example, label, index))
            {
                ex.Learn();
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
                    {
                        serializer.Value.Dispose();
                    }
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

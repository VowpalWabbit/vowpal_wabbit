// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitSerializerFactory.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;
using System.Reflection.Emit;
using VW.Serializer.Attributes;
using VW.Serializer.Interfaces;
using VW.Serializer.Intermediate;
using VW.Serializer.Reflection;
using VW.Serializer.Visitors;
using VW.Interfaces;
using System.IO;
using System.Runtime.CompilerServices;
using System.Diagnostics.Contracts;
using VW.Serializer.Inspectors;

namespace VW.Serializer
{
    /// <summary>
    /// Factory to ease creation of serializers.
    /// </summary>
    public static class VowpalWabbitSerializerFactory
    {

        /// <summary>
        /// Example and example result type based serializer cache.
        /// </summary>
        private static readonly Dictionary<Tuple<Type, Type>, object> SerializerCache = new Dictionary<Tuple<Type, Type>, object>();

        /// <summary>
        /// Compiles a serializers for the given example user type.
        /// </summary>
        /// <typeparam name="TExample">The example user type.</typeparam>
        /// <param name="settings">The serializer settings.</param>
        /// <returns>A serializer for the given user example type.</returns>
        public static VowpalWabbitSerializer<TExample> CreateSerializer<TExample>(VowpalWabbitSettings settings, List<FeatureExpression> allFeatures = null)
        {
            var serializer = CreateSerializer<TExample, VowpalWabbitInterfaceVisitor, VowpalWabbitExample>(allFeatures);

#if DEBUG
            var stringSerializer = CreateSerializer<TExample, VowpalWabbitStringVisitor, string>();

            Func<VowpalWabbit, TExample, ILabel, VowpalWabbitExample> wrappedSerializerFunc;
            if (serializer == null)
            {
                // if no features are found, no serializer is generated
                wrappedSerializerFunc = (_, __, ___) => null;
            }
            else
            {
                wrappedSerializerFunc = (vw, example, label) => new VowpalWabbitDebugExample(serializer.Result(vw, example, label), stringSerializer.Result(vw, example, label));
            }

            return new VowpalWabbitSerializer<TExample>(wrappedSerializerFunc, serializer.ResultExpression, settings)
            {
                StringSerializerExpression = stringSerializer.ResultExpression
            };
#else
            if (serializerFunc == null)
            {
                // if no features are found, no serializer is generated
                serializerFunc = (_,__,___) => null;
            }

            return new VowpalWabbitSerializer<TExample>(serializerFunc, serializer.ResultExpression, settings);
#endif
        }

        internal static VowpalWabbitSerializerCompiler<TExample, TVisitor, TExampleResult> CreateSerializer<TExample, TVisitor, TExampleResult>(List<FeatureExpression> allFeatures = null)
        {
            Tuple<Type, Type> cacheKey = null;
            if (allFeatures == null)
            {
                // if no feature mapping is provided, use [Feature] annotation on provided type.
                allFeatures = AnnotationInspector.ExtractFeatures(typeof(TExample)).ToList();

                cacheKey = Tuple.Create(typeof(TExample), typeof(TVisitor));
                object serializer;

                if (SerializerCache.TryGetValue(cacheKey, out serializer))
                {
                    return (VowpalWabbitSerializerCompiler<TExample, TVisitor, TExampleResult>)serializer;
                }
            }

            if (allFeatures.Count == 0)
            {
                return null;
            }

            var newSerializer = new VowpalWabbitSerializerCompiler<TExample, TVisitor, TExampleResult>(allFeatures);

            if (cacheKey != null)
            {
                SerializerCache[cacheKey] = newSerializer;
            }

            return newSerializer;
        }
    }
}

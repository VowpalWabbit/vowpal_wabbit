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
        private static readonly Dictionary<Type, object> SerializerCache = new Dictionary<Type, object>();

//        /// <summary>
//        /// Compiles a serializers for the given example user type.
//        /// </summary>
//        /// <typeparam name="TExample">The example user type.</typeparam>
//        /// <param name="settings">The serializer settings.</param>
//        /// <returns>A serializer for the given user example type.</returns>
//        public static VowpalWabbitSerializerCompiled<TExample> CreateSerializer<TExample>(VowpalWabbitSettings settings, List<FeatureExpression> allFeatures = null, List<Type> customFeaturizer = null)
//        {
//            var serializer = CreateSerializer<TExample>(allFeatures, customFeaturizer);

//            return new VowpalWabbitSerializer<TExample>(vw, serializer, settings);

////#if DEBUG
////            var stringSerializer = CreateSerializer<TExample, string>();

////            Func<VowpalWabbit, TExample, ILabel, VowpalWabbitExample> wrappedSerializerFunc;
////            if (serializer == null)
////            {
////                // if no features are found, no serializer is generated
////                wrappedSerializerFunc = (_, __, ___) => null;
////            }
////            else
////            {
////                if (stringSerializer != null)
////                {
////                    wrappedSerializerFunc = (vw, example, label) => new VowpalWabbitDebugExample(serializer.Result(vw, example, label), stringSerializer.Result(vw, example, label));
////                }
////                else
////                {
////                    wrappedSerializerFunc = (vw, example, label) => new VowpalWabbitDebugExample(serializer.Result(vw, example, label), string.Empty);
////                }
////            }

////            var ret = new VowpalWabbitSerializer<TExample>(wrappedSerializerFunc, serializer.ResultExpression, settings);

////            if (stringSerializer != null)
////            {
////                ret.StringSerializerExpression = stringSerializer.ResultExpression;
////            }

////            return ret;
////#else
////            if (serializerFunc == null)
////            {
////                // if no features are found, no serializer is generated
////                serializerFunc = (_,__,___) => null;
////            }

////            return new VowpalWabbitSerializer<TExample>(serializerFunc, serializer.ResultExpression, settings);
////#endif
//        }

        public static VowpalWabbitSerializerCompiled<TExample> CreateSerializer<TExample>(List<FeatureExpression> allFeatures = null, List<Type> customFeaturizer = null)
        {
            Type cacheKey = null;
            if (allFeatures == null)
            {
                // TOOD: enhance caching based on feature list & featurizer set
                // if no feature mapping is provided, use [Feature] annotation on provided type.
                allFeatures = AnnotationInspector.ExtractFeatures(typeof(TExample)).ToList();

                cacheKey = typeof(TExample);
                object serializer;

                if (SerializerCache.TryGetValue(cacheKey, out serializer))
                {
                    return (VowpalWabbitSerializerCompiled<TExample>)serializer;
                }
            }

            if (allFeatures.Count == 0)
            {
                return null;
            }

            var newSerializer = new VowpalWabbitSerializerCompiled<TExample>(allFeatures, customFeaturizer);

            if (cacheKey != null)
            {
                SerializerCache[cacheKey] = newSerializer;
            }

            return newSerializer;
        }
    }
}

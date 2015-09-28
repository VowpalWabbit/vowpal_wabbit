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
using VW.Serializer.Intermediate;
using VW.Interfaces;
using System.IO;
using System.Runtime.CompilerServices;
using System.Diagnostics.Contracts;

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

        public static VowpalWabbitSerializerCompiled<TExample> CreateSerializer<TExample>(VowpalWabbitSettings settings = null)
        {
            List<FeatureExpression> allFeatures = null;

            Type cacheKey = null;
            if (settings != null && settings.AllFeatures != null)
            {
                allFeatures = settings.AllFeatures;
            }
            else
            {
                cacheKey = typeof(TExample);
                object serializer;

                if (SerializerCache.TryGetValue(cacheKey, out serializer))
                {
                    return (VowpalWabbitSerializerCompiled<TExample>)serializer;
                }

                // TOOD: enhance caching based on feature list & featurizer set
                // if no feature mapping is provided, use [Feature] annotation on provided type.
                allFeatures = AnnotationInspector.ExtractFeatures(typeof(TExample)).ToList();
            }

            if (allFeatures == null || allFeatures.Count == 0)
            {
                return null;
            }

            var newSerializer = new VowpalWabbitSerializerCompiled<TExample>(allFeatures, settings == null ? null : settings.CustomFeaturizer);

            if (cacheKey != null)
            {
                SerializerCache[cacheKey] = newSerializer;
            }

            return newSerializer;
        }
    }
}

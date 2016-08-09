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
using VW.Labels;
using System.IO;
using System.Runtime.CompilerServices;
using System.Diagnostics.Contracts;
using VW.Reflection;
using System.Collections.ObjectModel;

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
        private static readonly Dictionary<Key, object> SerializerCache = new Dictionary<Key, object>();

        private sealed class Key
        {
            internal Type Type { get; set; }

            internal ITypeInspector TypeInspector { get; set; }

            internal bool EnableStringExampleGeneration { get; set; }

            internal bool EnableStringFloatCompact { get; set; }

            internal List<Type> CustomFeaturizer { get; set; }

            public override bool Equals(object obj)
            {
                var other = obj as Key;
                return other != null &&
                    this.Type == other.Type &&
                    this.TypeInspector == other.TypeInspector &&
                    this.EnableStringExampleGeneration == other.EnableStringExampleGeneration &&
                    this.EnableStringFloatCompact == other.EnableStringFloatCompact &&
                    ((this.CustomFeaturizer == null && other.CustomFeaturizer == null) || this.CustomFeaturizer.SequenceEqual(other.CustomFeaturizer));
            }

            public override int GetHashCode()
            {
                return this.Type.GetHashCode() ^
                    this.TypeInspector.GetHashCode() ^
                    this.EnableStringExampleGeneration.GetHashCode() ^
                    this.EnableStringFloatCompact.GetHashCode() ^
                    (this.CustomFeaturizer == null ? 1 : this.CustomFeaturizer.GetHashCode());
            }
        }

        /// <summary>
        /// Creates a serializer for the given type and settings.
        /// </summary>
        /// <typeparam name="TExample">The user type to serialize.</typeparam>
        /// <param name="settings"></param>
        /// <returns></returns>
        public static IVowpalWabbitSerializerCompiler<TExample> CreateSerializer<TExample>(VowpalWabbitSettings settings = null)
        {
            Schema schema = null;

            Key cacheKey = null;
            if (settings != null && settings.Schema != null)
            {
                schema = settings.Schema;
            }
            else
            {
                ITypeInspector typeInspector = settings.TypeInspector;
                if (typeInspector == null)
                    typeInspector = TypeInspector.Default;

                // only cache non-string generating serializer
                cacheKey = new Key
                {
                    Type = typeof(TExample),
                    TypeInspector = typeInspector,
                    CustomFeaturizer = settings == null ? null : settings.CustomFeaturizer,
                    EnableStringExampleGeneration = settings == null ? false : settings.EnableStringExampleGeneration,
                    EnableStringFloatCompact = settings == null ? false : settings.EnableStringFloatCompact
                };

                object serializer;
                if (SerializerCache.TryGetValue(cacheKey, out serializer))
                {
                    return (IVowpalWabbitSerializerCompiler<TExample>)serializer;
                }

                
                // TODO: enhance caching based on feature list & featurizer set
                // if no feature mapping is provided, use [Feature] annotation on provided type.
                schema = typeInspector.CreateSchema(settings, typeof(TExample));

                var multiExampleSerializerCompiler = VowpalWabbitMultiExampleSerializerCompiler.TryCreate<TExample>(settings, schema);
                if (multiExampleSerializerCompiler != null)
                    return multiExampleSerializerCompiler;
            }

            // need at least a single feature to do something sensible
            if (schema == null || schema.Features.Count == 0)
            {
                return null;
            }

            var newSerializer = new VowpalWabbitSingleExampleSerializerCompiler<TExample>(
                schema,
                settings == null ? null : settings.CustomFeaturizer,
                !settings.EnableStringExampleGeneration);

            if (cacheKey != null)
            {
                SerializerCache[cacheKey] = newSerializer;
            }

            return newSerializer;
        }
    }
}

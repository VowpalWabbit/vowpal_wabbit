// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitJsonOptimizedSerializable.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using Newtonsoft.Json;
using System;
using System.IO;
using VW.Serializer.Intermediate;

namespace VW.Serializer
{
    /// <summary>
    /// Uses the supplied <see cref="IVowpalWabbitJsonConverter"/> to get the JSON fragments for a given object.
    /// </summary>
    public class VowpalWabbitJsonOptimizedSerializable : IVowpalWabbitSerializable
    {
        private readonly object value;
        private readonly IVowpalWabbitJsonConverter jsonConverter;

        /// <summary>
        /// Initializes a new <see cref="VowpalWabbitJsonOptimizedSerializable"/> instance.
        /// </summary>
        public VowpalWabbitJsonOptimizedSerializable(object value, IVowpalWabbitJsonConverter jsonConverter)
        {
            this.value = value;
            this.jsonConverter = jsonConverter;
        }

        /// <summary>
        /// Marshals JSON string into VW example.
        /// </summary>
        public void Marshal(VowpalWabbitMarshalContext ctx, Namespace ns, Feature feature)
        {
            if (this.value == null)
                return;

            try
            {
                var jsonSerializer = new JsonSerializer();
                using (var jsonBuilder = new VowpalWabbitJsonBuilder(ctx.VW, VowpalWabbitDefaultMarshaller.Instance, jsonSerializer))
                {
                    // marshal from JSON to VW
                    foreach (var json in jsonConverter.JsonFragments(this.value))
                    {
                        if (json == null)
                            continue;

                        using (var reader = new JsonTextReader(new StringReader(json)))
                        {
                            jsonBuilder.Parse(reader, ctx, new Namespace(ctx.VW, feature.Name));
                        }
                    }
                }
            }
            catch (Exception e)
            {
                throw new VowpalWabbitSerializationException("Optimized marshalling failed", e, ns, feature);
            }
        }
    }
}

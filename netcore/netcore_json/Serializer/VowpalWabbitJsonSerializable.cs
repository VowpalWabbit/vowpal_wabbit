// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitJsonSerializable.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW.Serializer.Intermediate;

namespace VW.Serializer
{
    /// <summary>
    /// Uses supplied JsonConverter to custom serialize object into JSON and then marshal from there.
    /// </summary>
    public class VowpalWabbitJsonSerializable : IVowpalWabbitSerializable
    {
        private readonly object value;
        private readonly JsonConverter jsonConverter;

        /// <summary>
        /// Initializes a new <see cref="VowpalWabbitJsonSerializable"/> instance.
        /// </summary>
        public VowpalWabbitJsonSerializable(object value, JsonConverter jsonConverter)
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

            var jsonSerializer = new JsonSerializer();
            using (var jsonBuilder = new VowpalWabbitJsonBuilder(ctx.VW, VowpalWabbitDefaultMarshaller.Instance, jsonSerializer))
            {
                // serialize from object to JSON
                var sb = new StringBuilder();
                using (var writer = new JsonTextWriter(new StringWriter(sb)))
                {
                    this.jsonConverter.WriteJson(writer, this.value, jsonSerializer);
                }

                // marshal from JSON to VW
                using (var reader = new JsonTextReader(new StringReader(sb.ToString())))
                {
                    jsonBuilder.Parse(reader, ctx, new Namespace(ctx.VW, feature.Name));
                }
            }
        }
    }
}

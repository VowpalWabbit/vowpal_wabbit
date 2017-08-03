// --------------------------------------------------------------------------------------------------------------------
// <copyright file="JsonRawStringListConverter.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using Newtonsoft.Json;
using System;
using System.Collections.Generic;

namespace VW.Serializer
{
    /// <summary>
    /// Custom JSON converter returning the underlying raw json (avoiding object allocation).
    /// </summary>
    public class JsonRawStringListConverter : JsonConverter, IVowpalWabbitJsonConverter
    {
        /// <summary>
        /// Supports string only.
        /// </summary>
        public override bool CanConvert(Type objectType)
        {
            return objectType == typeof(List<string>);
        }

        /// <summary>
        /// Not implemented.
        /// </summary>
        public override object ReadJson(JsonReader reader, Type objectType, object existingValue, JsonSerializer serializer)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// Outputs the string contents as JSON.
        /// </summary>
        public override void WriteJson(JsonWriter writer, object value, JsonSerializer serializer)
        {
            var valueStringEnumerable = value as List<string>;
            if (valueStringEnumerable != null)
            {
                writer.WriteStartArray();
                foreach (var str in valueStringEnumerable)
                    writer.WriteRawValue(str);
                writer.WriteEndArray();
                return;
            }

            serializer.Serialize(writer, value);
        }

        /// <summary>
        /// List of independently parseable JSON fragments.
        /// </summary>
        public IEnumerable<string> JsonFragments(object value)
        {
            var valueStringList = value as List<string>;
            if (valueStringList == null)
                throw new ArgumentException($"Unsupported type: {value}");

            return valueStringList;
        }
    }
}

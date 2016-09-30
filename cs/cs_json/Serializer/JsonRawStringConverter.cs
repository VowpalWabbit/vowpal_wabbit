// --------------------------------------------------------------------------------------------------------------------
// <copyright file="JsonRawStringConverter.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VW.Serializer
{
    /// <summary>
    /// Custom JSON converter returning the underlying raw json (avoiding object allocation).
    /// </summary>
    public class JsonRawStringConverter : JsonConverter
    {
        /// <summary>
        /// Supports string only.
        /// </summary>
        public override bool CanConvert(Type objectType)
        {
            return objectType == typeof(string);
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
            var valueString = value as string;
            if (valueString != null)
                writer.WriteRawValue(valueString);
            else
                serializer.Serialize(writer, value);
        }
    }
}

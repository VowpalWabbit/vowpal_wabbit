// --------------------------------------------------------------------------------------------------------------------
// <copyright file="PrefixedJsonReader.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;

namespace VW.Serializer
{
    /// <summary>
    /// A Json Reader allowing to prefix data from a wrapped JsonReader.
    /// </summary>
    internal class PrefixedJsonReader : JsonReader
    {
        private JsonReader reader;
        private Stack<Tuple<JsonToken, object>> prefix;

        internal PrefixedJsonReader(JsonReader reader, params Tuple<JsonToken, object>[] prefix)
        {
            this.reader = reader;
            this.prefix = new Stack<Tuple<JsonToken, object>>(prefix.Reverse());
        }

        public override bool Read()
        {
            if (this.prefix.Count > 0)
            {
                var t = prefix.Pop();
                this.SetToken(t.Item1, t.Item2);
                return true;
            }

            if (!this.reader.Read())
                return false;

            this.SetToken(this.reader.TokenType, this.reader.Value);

            return true;
        }
    }
}

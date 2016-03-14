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
        private Queue<Tuple<JsonToken, object>> prefix;

        /// <summary>
        /// Initializes a new instance of <see cref="PrefixedJsonReader"/>.
        /// </summary>
        /// <param name="reader">The reader to be wrapped.</param>
        /// <param name="prefix">The JsonTokens to be injected at the beginning of the stream.</param>
        internal PrefixedJsonReader(JsonReader reader, params Tuple<JsonToken, object>[] prefix)
        {
            this.reader = reader;
            this.prefix = new Queue<Tuple<JsonToken, object>>(prefix);
        }

        /// <summary>
        /// Injects the supplied prefix into the stream.
        /// </summary>
        /// <returns>True if another token is available, false otherwise.</returns>
        public override bool Read()
        {
            if (this.prefix.Count > 0)
            {
                var t = prefix.Dequeue();
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

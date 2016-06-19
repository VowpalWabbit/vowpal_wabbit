// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitJsonException.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using Newtonsoft.Json;
using System;

namespace VW.Serializer
{
    /// <summary>
    /// Exception thrown if <see cref="VowpalWabbitJsonSerializer"/> fails to deserialize the JSON.
    /// </summary>
    [Serializable]
    public sealed class VowpalWabbitJsonException : Exception
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="VowpalWabbitJsonException"/> class.
        /// </summary>
        /// <param name="reader">The reader used at deserialization time.</param>
        /// <param name="message">The message that describes the error.</param>
        public VowpalWabbitJsonException(JsonReader reader, string message)
             : base(message)
        {
            this.Path = reader.Path;

            var lineInfo = reader as IJsonLineInfo;
            if (lineInfo != null)
            {
                this.LineNumber = lineInfo.LineNumber;
                this.LinePosition = lineInfo.LinePosition;
            }
        }

        /// <summary>
        /// The line number at which this error happened.
        /// </summary>
        public int LineNumber { get; private set; }

        /// <summary>
        /// The character position at which this error happened.
        /// </summary>
        public int LinePosition { get; private set; }

        /// <summary>
        /// The path as returned by <see cref="Newtonsoft.Json.JsonReader.Path"/>.
        /// </summary>
        public string Path { get; private set; }
    }
}

// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitJsonException.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;

namespace VW.Serializer
{
    /// <summary>
    /// Exception thrown if <see cref="VowpalWabbitJsonSerializer"/> fails to deserialize the JSON.
    /// </summary>
    [Serializable]
    public class VowpalWabbitJsonException : Exception
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="VowpalWabbitJsonException"/> class.
        /// </summary>
        /// <param name="path">The path as returned by <see cref="Newtonsoft.Json.JsonReader.Path"/>.</param>
        /// <param name="message">The message that describes the error.</param>
        internal VowpalWabbitJsonException(string path, string message)
             : base(message)
        {
            this.Path = path;
        }

        /// <summary>
        /// The path as returned by <see cref="Newtonsoft.Json.JsonReader.Path"/>.
        /// </summary>
        public string Path { get; private set; }
    }
}

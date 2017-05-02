// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitSerializationException.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using VW.Serializer.Intermediate;

namespace VW
{
    /// <summary>
    /// Exception thrown while serialization.
    /// </summary>
    public class VowpalWabbitSerializationException : Exception
    {
        /// <summary>
        /// Constructs new exception
        /// </summary>
        public VowpalWabbitSerializationException(string message, Exception innerException, Namespace ns, Feature feature)
            : base($"{message}. Namespace: {ns.Name}. Feature: {feature.Name}", innerException)
        {
            this.Namespace = ns;
            this.Feature = feature;
        }

        /// <summary>
        /// The related namespace for this exception.
        /// </summary>
        public Namespace Namespace { get; private set; }

        /// <summary>
        /// The related feature for this feature.
        /// </summary>
        public Feature Feature { get; private set; }
    }
}

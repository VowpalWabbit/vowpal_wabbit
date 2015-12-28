// --------------------------------------------------------------------------------------------------------------------
// <copyright file="Namespace.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Globalization;

namespace VW.Serializer.Intermediate
{
    /// <summary>
    /// The intermediate namespace representation.
    /// </summary>
    public sealed class Namespace
    {
        /// <summary>
        /// Initializes a new namespace instance.
        /// </summary>
        /// <param name="vw">VopwpalWabbit instance used for hashing.</param>
        /// <param name="name">The namespace name.</param>
        /// <param name="featureGroup">Defaults to space, if null.</param>
        public Namespace(VowpalWabbit vw, string name, char? featureGroup)
        {
            this.Name = name;
            this.FeatureGroup = featureGroup ?? ' ';

            if (featureGroup == null && !string.IsNullOrWhiteSpace(name))
            {
                throw new ArgumentException("If Namespace is provided, FeatureGroup must be set too");
            }

            // compute shared namespace hash
            this.NamespaceHash = name == null ?
                vw.HashSpace(this.FeatureGroup.ToString()) :
                vw.HashSpace(this.FeatureGroup + this.Name);

            this.NamespaceString = string.Format(
                CultureInfo.InvariantCulture,
                " |{0}{1}",
                this.FeatureGroup,
                this.Name);
        }

        /// <summary>
        /// Gets or sets the namespace name.
        /// </summary>
        public string Name { get; private set; }

        /// <summary>
        /// Gets or sets the feature group.
        /// </summary>
        public char FeatureGroup { get; private set; }

        /// <summary>
        /// The pre-calculated hash.
        /// </summary>
        public ulong NamespaceHash { get; private set; }

        /// <summary>
        /// The string representation of the namespace.
        /// </summary>
        public string NamespaceString { get; private set; }
    }
}

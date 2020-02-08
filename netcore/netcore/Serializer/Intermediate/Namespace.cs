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
        /// Initializes a new <see cref="Namespace"/> instance.
        /// </summary>
        /// <param name="vw">VopwpalWabbit instance used for hashing.</param>
        /// <param name="name">The namespace name.</param>
        /// <param name="featureGroup">Defaults to space, if null.</param>
        public Namespace(VowpalWabbit vw, string name, char? featureGroup)
        {
            this.Name = name;
            this.FeatureGroup = featureGroup ?? VowpalWabbitConstants.DefaultNamespace;

            if (featureGroup == null && !string.IsNullOrWhiteSpace(name))
            {
                throw new ArgumentException("If Namespace is provided, FeatureGroup must be set too");
            }

            // compute shared namespace hash
            this.NamespaceHash = name == null ?
                vw.HashSpace(this.FeatureGroup.ToString()) :
                vw.HashSpace(this.FeatureGroup + this.Name);

            if (vw.Settings.EnableStringExampleGeneration)
                this.NamespaceString = string.Format(
                    CultureInfo.InvariantCulture,
                    " |{0}{1}",
                    this.FeatureGroup,
                    this.Name);
        }

        /// <summary>
        /// Initializes a new <see cref="Namespace"/> instance.
        /// </summary>
        /// <param name="vw">VopwpalWabbit instance used for hashing.</param>
        /// <param name="name">The namespace name. First character is treated as feature group. Defaults to space.</param>
        public Namespace(VowpalWabbit vw, string name = null)
        {
            if (string.IsNullOrWhiteSpace(name))
                name = VowpalWabbitConstants.DefaultNamespace.ToString();

            if (name.Length > 1)
                this.Name = name.Substring(1);

            this.FeatureGroup = name[0];

            this.NamespaceHash = vw.HashSpace(name);

            if (vw.Settings.EnableStringExampleGeneration)
                this.NamespaceString = " |" + name;
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

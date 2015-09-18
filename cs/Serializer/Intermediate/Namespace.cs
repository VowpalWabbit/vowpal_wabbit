using System.Globalization;
// --------------------------------------------------------------------------------------------------------------------
// <copyright file="Namespace.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------
using VW.Serializer.Interfaces;

namespace VW.Serializer.Intermediate
{
    /// <summary>
    /// The intermediate namespace representation.
    /// </summary>
    public sealed class Namespace // : INamespace
    {
        public Namespace(VowpalWabbit vw, string name, char? featureGroup)
        {
            this.Name = name;
            this.FeatureGroup = featureGroup ?? ' ';

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

        public uint NamespaceHash { get; private set; }

        public string NamespaceString { get; private set; }
    }
}

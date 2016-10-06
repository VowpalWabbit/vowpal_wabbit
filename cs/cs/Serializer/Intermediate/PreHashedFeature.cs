// --------------------------------------------------------------------------------------------------------------------
// <copyright file="PreHashedFeature.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace VW.Serializer.Intermediate
{
    /// <summary>
    /// The intermediate feature representation.
    /// </summary>
    public sealed class PreHashedFeature : Feature
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="PreHashedFeature"/> class.
        /// </summary>
        /// <param name="vw">The vowpal wabbit instance.</param>
        /// <param name="ns">The namespace.</param>
        /// <param name="name">The feature name/</param>
        /// <param name="addAnchor">True if an anchor needs to be added, false otherwise.</param>
        /// <param name="dictify"></param>
        public PreHashedFeature(VowpalWabbit vw, Namespace ns, string name, bool addAnchor = false, bool dictify = false)
            : base(name, addAnchor, dictify)
        {
            this.FeatureHash = vw.HashFeature(this.Name, ns.NamespaceHash);
        }

        /// <summary>
        /// The pre-hashed feature hash.
        /// </summary>
        public ulong FeatureHash { get; private set; }
    }
}

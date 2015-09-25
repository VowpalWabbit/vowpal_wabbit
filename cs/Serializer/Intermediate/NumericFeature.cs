// --------------------------------------------------------------------------------------------------------------------
// <copyright file="NumericFeature.cs">
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
    public sealed class NumericFeature : Feature
    {
        public NumericFeature(VowpalWabbit vw, Namespace ns, string name, bool addAnchor)
            : base(name, addAnchor)
        {
            this.FeatureHash = vw.HashFeature(this.Name, ns.NamespaceHash);
        }

        public uint FeatureHash { get; private set; }
    }
}

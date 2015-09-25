// --------------------------------------------------------------------------------------------------------------------
// <copyright file="EnumerizedFeature.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;

namespace VW.Serializer.Intermediate
{
    public sealed class EnumerizedFeature<T> : Feature
    {
        private VowpalWabbit vw;
        private Namespace ns;
        private Func<T, uint> enumHashing;

        public EnumerizedFeature(VowpalWabbit vw, Namespace ns, string name, bool addAnchor, Func<EnumerizedFeature<T>, Func<T, uint>> enumHashing)
            : base(name, addAnchor)
        {
            if (!typeof(T).IsEnum)
            {
                throw new ArgumentException(string.Format("Type {0} must be enum", typeof(T)));
            }

            this.vw = vw;
            this.ns = ns;
            this.enumHashing = enumHashing(this);
        }

        public uint FeatureHash(T value)
        {
            return this.enumHashing(value);
        }

        public uint FeatureHashInternal(T value)
        {
            return this.vw.HashFeature(
                this.Name + Enum.GetName(typeof(T), value),
                this.ns.NamespaceHash);
        }
    }
}

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
    /// <summary>
    /// Feature description for enumerized features. Instead of estimating a single parameter/weight
    /// for a given integer, VW will estimate a parameter/weight for each value (one-hot encoding, dummy variables)
    /// </summary>
    /// <typeparam name="T">The value type.</typeparam>
    public sealed class EnumerizedFeature<T> : Feature
    {
        private readonly VowpalWabbit vw;
        private readonly Namespace ns;

        /// <summary>
        /// Enum hashing function.
        /// </summary>
        private readonly Func<T, ulong> enumHashing;

        /// <summary>
        /// Initializes a new EnumerizedFeature.
        /// </summary>
        /// <param name="vw">Vowpal Wabbit instance required for hashing.</param>
        /// <param name="ns">The associated namespace.</param>
        /// <param name="name">The feature name.</param>
        /// <param name="addAnchor"></param>
        /// <param name="dictify">If true, enable dictionary extraction.</param>
        /// <param name="enumHashing">The enumHash cache factory.</param>
        public EnumerizedFeature(VowpalWabbit vw, Namespace ns, string name, bool addAnchor, bool dictify, Func<EnumerizedFeature<T>, Func<T, ulong>> enumHashing)
            : base(name, addAnchor, dictify)
        {
            if (!typeof(T).IsEnum)
            {
                throw new ArgumentException(string.Format("Type {0} must be enum", typeof(T)));
            }

            this.vw = vw;
            this.ns = ns;

            // initialize the enumHashing function
            this.enumHashing = enumHashing(this);
        }

        /// <summary>
        /// Hashes <paramref name="value"/> potentially using a cache.
        /// </summary>
        /// <param name="value">The value to be hashed.</param>
        /// <returns>The hash of <see cref="Feature.Name"/> + <paramref name="value"/></returns>
        public ulong FeatureHash(T value)
        {
            return this.enumHashing(value);
        }

        /// <summary>
        /// Hashes <paramref name="value"/> directly (no caching).
        /// </summary>
        /// <param name="value">The value to be hashed.</param>
        /// <returns>The hash of <see cref="Feature.Name"/> + <paramref name="value"/></returns>
        public ulong FeatureHashInternal(T value)
        {
            return this.vw.HashFeature(
                this.Name + Enum.GetName(typeof(T), value),
                this.ns.NamespaceHash);
        }
    }
}

// --------------------------------------------------------------------------------------------------------------------
// <copyright file="Feature.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Linq.Expressions;
using VW.Serializer.Interfaces;

namespace VW.Serializer.Intermediate
{
    public class Feature
    {
        public Feature(string name, bool addAnchor)
        {
            if (name == null)
            {
                throw new ArgumentNullException("name");
            }

            this.Name = name;
            this.AddAnchor = addAnchor;
            // TODO: this.Name += Convert.ToString(feature.Value);
        }

        /// <summary>
        /// The origin property name is used as the feature name.
        /// </summary>
        public string Name { get; private set; }

        /// <summary>
        /// If true, an anchoring feature (0:1) will be inserted at front.
        /// This is required if --interact is used to mark the beginning of the feature namespace,
        /// as 0-valued features are removed.
        /// </summary>
        /// <remarks>Defaults to false.</remarks>
        public bool AddAnchor { get; private set; }
    }

    /// <summary>
    /// The intermediate feature representation.
    /// </summary>
    public sealed class NumericFeature : Feature // : IFeature
    {
        public NumericFeature(VowpalWabbit vw, Namespace ns, string name, bool addAnchor)
            : base(name, addAnchor)
        {
            this.FeatureHash = vw.HashFeature(this.Name, ns.NamespaceHash);
            // TODO: this.Name += Convert.ToString(feature.Value);
        }

        ///// If true, features will be converted to string and then hashed.
        ///// In VW line format: Age:15 (Enumerize=false), Age_15 (Enumerize=true)
        ///// Defaults to false.
        ///// </summary>
        //public bool Enumerize { get; private set; }



        public uint FeatureHash { get; private set; }
    }

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
/*
    public sealed class Feature : BaseFeature
    {
        private readonly VowpalWabbit vw;

        private readonly Namespace ns;

        public EnumerizedFeature(VowpalWabbit vw, Namespace ns, string name, bool addAnchor)
            : base(name, addAnchor)
        {
            this.vw = vw;
            this.ns = ns;
        }

        public uint FeatureHash<T>(T value)
        {
            return this.vw.HashFeature(this.Name + Convert.ToString(value), this.ns.NamespaceHash);
        }
    }



    ///// <summary>
    ///// The typed representation of the feature.
    ///// </summary>
    ///// <typeparam name="T">Type of feature value.</typeparam>
    //public sealed class Feature<T> : IFeature<T>, IVisitableFeature // Feature,
    //{
    //    /// <summary>
    //    /// The actual value
    //    /// </summary>
    //    public T Value { get; set; }

    //    /// <summary>
    //    /// Compiled func to enable automatic double dispatch.
    //    /// </summary>
    //    public Action Visit { get; set; }
    //}
 */
}

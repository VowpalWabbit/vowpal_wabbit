// --------------------------------------------------------------------------------------------------------------------
// <copyright file="FeatureAttribute.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using VW.Serializer;

namespace VW.Serializer.Attributes
{
    /// <summary>
    /// Annotate properties that should be serialized to Vowpal Wabbit
    /// </summary>
    [AttributeUsage(AttributeTargets.Property)]
    public sealed class FeatureAttribute : Attribute
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="FeatureAttribute"/> class.
        /// </summary>
        public FeatureAttribute()
        {
            this.Enumerize = false;
            this.AddAnchor = false;
            this.StringProcessing = StringProcessing.Split;
        }

        /// <summary>
        /// The namespace. Corresponds to the string literal after | in the native VW line format. <br/>
        /// e.g. user in "|user :0.1 :0.2"
        /// </summary>
        public string Namespace { get; set; }

        /// <summary>
        /// If true, features will be converted to string and then hashed.
        /// In VW line format: Age:15 (Enumerize=false), Age_15 (Enumerize=true)
        /// </summary>
        /// <remarks>Defaults to false.</remarks>
        public bool Enumerize { get; set; }

        /// <summary>
        /// If true, an anchoring feature (0:1) will be inserted at front.
        /// This is required if --interact is used to mark the beginning of the feature namespace,
        /// as 0-valued features are removed.
        /// </summary>
        /// <remarks>Defaults to false.</remarks>
        public bool AddAnchor { get; set; }

        /// <summary>
        /// If true, the string serialization will collect the feature into a dictionary and output a surrogate.
        /// </summary>
        /// <remarks>Defaults to null, which inherits from parent. If no parent information available, defaults to false.</remarks>
        public bool Dictify
        {
            get { return InternalDictify ?? false; }
            set { this.InternalDictify = value; }
        }

        /// <summary>
        /// Cope with potential null values.
        /// </summary>
        internal bool? InternalDictify { get; set; }

        /// <summary>
        /// Cope with potential null values.
        /// </summary>
        internal char? InternalFeatureGroup { get; set; }

        /// <summary>
        /// The regular VW string interface interprets the first character of the namespace as the feature group.
        /// </summary>
        public char FeatureGroup
        {
            get { return InternalFeatureGroup ?? VowpalWabbitConstants.DefaultNamespace; }
            set { this.InternalFeatureGroup = value; }
        }

        /// <summary>
        /// Allows feature name override.
        /// </summary>
        /// <remarks>Defaults to reflected property name.</remarks>
        public string Name { get; set; }

        /// <summary>
        /// Specify the serialization order.
        /// </summary>
        public int Order { get; set; }

        /// <summary>
        /// Configures string pre-processing before hashing. All options are compatible with VW string format.
        /// </summary>
        /// <remarks>Defaults to <see cref="VW.Serializer.StringProcessing.Split"/></remarks>
        public StringProcessing StringProcessing { get; set; }
    }
}

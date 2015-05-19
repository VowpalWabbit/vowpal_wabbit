// --------------------------------------------------------------------------------------------------------------------
// <copyright file="FeatureAttribute.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------
using System;

namespace Microsoft.Research.MachineLearning.Serializer.Attributes
{
    /// <summary>
    /// Annotate properties that should be serialized to Vowpal Wabbit
    /// </summary>
    [AttributeUsage(AttributeTargets.Property)]
    public class FeatureAttribute : Attribute
    {
        public FeatureAttribute()
        {
            this.Enumerize = false;
        }

        /// <summary>
        /// The namespace. Corresponds to the string literal after | in the native VW line format. <br/>
        /// e.g. user in "|user :0.1 :0.2"
        /// </summary>
        public string Namespace { get; set; }

        /// <summary>
        /// If true, features will be converted to string and then hashed.
        /// In VW line format: Age:15 (Enumerize=false), Age_15 (Enumerize=true)
        /// Defaults to false.
        /// </summary>
        public bool Enumerize { get; set; }

        /// <summary>
        /// Cope with potential null values.
        /// </summary>
        internal char? InternalFeatureGroup { get; set; }

        /// <summary>
        /// The regular VW string interface interprets the first character of the namespace as the feature group.
        /// </summary>
        public char FeatureGroup
        {
            // TODO: is ' ' a good default?
            get { return InternalFeatureGroup ?? ' '; }
            set { this.InternalFeatureGroup = value; }
        }
    }
}

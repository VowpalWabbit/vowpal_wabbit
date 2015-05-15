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
    [AttributeUsage(AttributeTargets.Property)]
    public class FeatureAttribute : Attribute
    {
        public FeatureAttribute()
        {
            this.Enumerize = false;
        }

        public Type Converter { get; set; }

        public string Namespace { get; set; }

        public bool Enumerize { get; set; }

        internal char? InternalFeatureGroup { get; set; }

        public char FeatureGroup
        {
            get { return InternalFeatureGroup ?? ' '; }
            set { this.InternalFeatureGroup = value; }
        }
    }
}

// --------------------------------------------------------------------------------------------------------------------
// <copyright file="Feature.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Diagnostics;
using System.Diagnostics.Contracts;
using System.Linq.Expressions;

namespace VW.Serializer.Intermediate
{
    /// <summary>
    /// The base feature description.
    /// </summary>
    [DebuggerDisplay("Feature({Name}, addAnchor: {AddAnchor}, dictify: {Dictify}")]
    public class Feature
    {
        /// <summary>
        /// Initializes a new Feature.
        /// </summary>
        /// <param name="name"></param>
        /// <param name="addAnchor"></param>
        /// <param name="dictify"></param>
        public Feature(string name, bool addAnchor = false, bool dictify = false)
        {
            this.Name = name;
            this.AddAnchor = addAnchor;
            this.Dictify = dictify;
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

        /// <summary>
        /// If true, the string serialization will collect the feature into a dictionary and output a surrogate.
        /// </summary>
        /// <remarks>Defaults to null, which inherits from parent. If no parent information available, defaults to false.</remarks>
        public bool Dictify { get; private set; }
    }
}

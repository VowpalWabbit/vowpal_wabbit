// --------------------------------------------------------------------------------------------------------------------
// <copyright file="Feature.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Linq.Expressions;

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
}

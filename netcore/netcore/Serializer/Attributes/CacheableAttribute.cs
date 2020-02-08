// --------------------------------------------------------------------------------------------------------------------
// <copyright file="CachableAttribute.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;

namespace VW.Serializer.Attributes
{
    /// <summary>
    /// Annotate an example class that's generated output should be cached.
    /// </summary>
    [AttributeUsage(AttributeTargets.Class)]
    public sealed class CacheableAttribute : Attribute
    {
        /// <summary>
        /// Specify an equality comparer to be used for the dictionary cache.
        /// If non is specified, default behavior of the <see cref="System.Collections.Generic.Dictionary{K,V}"/>
        /// </summary>
        public Type EqualityComparer { get; set; }
    }
}

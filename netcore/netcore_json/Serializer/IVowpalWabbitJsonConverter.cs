// --------------------------------------------------------------------------------------------------------------------
// <copyright file="IVowpalWabbitJsonConverter.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System.Collections.Generic;

namespace VW.Serializer
{
    /// <summary>
    /// Optimization interface for JsonConverter holding one or more independently parseable JSON fragments.
    /// </summary>
    /// <remarks>
    /// This avoids string copying.
    /// </remarks>
    public interface IVowpalWabbitJsonConverter
    {
        /// <summary>
        /// List of independently parseable JSON fragments.
        /// </summary>
        IEnumerable<string> JsonFragments(object value);
    }
}

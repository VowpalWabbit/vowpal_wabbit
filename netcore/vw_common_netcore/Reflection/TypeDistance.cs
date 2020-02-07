// --------------------------------------------------------------------------------------------------------------------
// <copyright file="TypeDistance.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;

namespace VW.Reflection
{
    /// <summary>
    /// Models a distance to a given type.
    /// </summary>
    internal sealed class TypeDistance
    {
        internal int Distance { get; set; }

        internal Type Type { get; set; }
    }
}

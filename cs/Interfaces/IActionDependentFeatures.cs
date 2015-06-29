// --------------------------------------------------------------------------------------------------------------------
// <copyright file="IActionDependentFeatureExample.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System.Collections.Generic;

namespace VW.Interfaces
{
    /// <summary>
    /// Types supporting action dependent features must implement this interface.
    /// </summary>
    /// <typeparam name="T">Type of each action dependent feature.</typeparam>
    public interface IActionDependentFeatureExample<out T>
    {
        IReadOnlyList<T> ActionDependentFeatures { get; }
    }
}

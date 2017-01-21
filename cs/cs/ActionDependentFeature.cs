// --------------------------------------------------------------------------------------------------------------------
// <copyright file="ActionDependentFeature.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace VW
{
  /// <summary>
  /// A tuple of an action dependent feature and the corresponding index.
  /// </summary>
  /// <typeparam name="TActionDependentFeature">The action dependent feature type.</typeparam>
  public sealed class ActionDependentFeature<TActionDependentFeature>
  {
    internal ActionDependentFeature(int index, TActionDependentFeature feature)
    {
      this.Index = index;
      this.Feature = feature;
    }

    /// <summary>
    /// The index within the multi-line example.
    /// </summary>
    public int Index { get; private set; }


    /// <summary>
    /// The index within the multi-line example.
    /// </summary>
    public float Probability { get; set; }

    /// <summary>
    /// The feature object.
    /// </summary>
    public TActionDependentFeature Feature { get; private set; }
  }
}

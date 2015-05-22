// --------------------------------------------------------------------------------------------------------------------
// <copyright file="ILabel.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace Microsoft.Research.MachineLearning.Interfaces
{
    public interface ILabel
    {
        string ToVowpalWabbitFormat();
    }
}

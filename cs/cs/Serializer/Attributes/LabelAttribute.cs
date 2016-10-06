// --------------------------------------------------------------------------------------------------------------------
// <copyright file="LabelAttribute.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;

namespace VW.Serializer.Attributes
{
    /// <summary>
    /// Used to annotate properties designated as labels.
    /// </summary>
    [AttributeUsage(AttributeTargets.Property)]
    public sealed class LabelAttribute : Attribute
    {
    }
}

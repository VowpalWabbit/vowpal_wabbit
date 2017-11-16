// --------------------------------------------------------------------------------------------------------------------
// <copyright file="IVowpalWabbitSerializable.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using VW.Serializer.Intermediate;

namespace VW.Serializer
{
    /// <summary>
    /// Types implementing custom serialization to VW should implement this interface.
    /// </summary>
    public interface IVowpalWabbitSerializable
    {
        /// <summary>
        /// Marshals this object into native VW
        /// </summary>
        /// <param name="ctx"></param>
        /// <param name="ns"></param>
        /// <param name="feature"></param>
        void Marshal(VowpalWabbitMarshalContext ctx, Namespace ns, Feature feature);
    }
}

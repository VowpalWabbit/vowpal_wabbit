// --------------------------------------------------------------------------------------------------------------------
// <copyright file="IVowpalWabbitSerializerCompiler.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace VW.Serializer
{
    /// <summary>
    /// Abstraction for single vs. multiline examples.
    /// </summary>
    /// <typeparam name="TExample">The user-defined type to be serialized.</typeparam>
    public interface IVowpalWabbitSerializerCompiler<TExample>
    {
        /// <summary>
        /// Creates a new serializer for the given type.
        /// </summary>
        /// <param name="vw">The VW instance this serializer is associated with.</param>
        /// <returns>A ready to use serializer.</returns>
        IVowpalWabbitSerializer<TExample> Create(VowpalWabbit vw);
    }
}

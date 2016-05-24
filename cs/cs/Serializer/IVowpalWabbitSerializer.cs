// --------------------------------------------------------------------------------------------------------------------
// <copyright file="IVowpalWabbitSerializer.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using VW.Labels;

namespace VW.Serializer
{
    /// <summary>
    /// Abstraction for single vs. multiline examples.
    /// </summary>
    /// <typeparam name="TExample">The user-defined type to be serialized.</typeparam>
    public interface IVowpalWabbitSerializer<TExample> : IDisposable
    {
        /// <summary>
        /// True if Vowpal Wabbit strings are generated, false otherwise.
        /// </summary>
        bool EnableStringExampleGeneration { get; }

        /// <summary>
        /// True if this serializer caches examples, false otherwise.
        /// </summary>
        bool CachesExamples { get; }

        /// <summary>
        /// Serializes the given <paramref name="example"/> into a Vowpal Wabbit string.
        /// </summary>
        /// <param name="example">The example to serialize.</param>
        /// <param name="label">The optional label to serialize.</param>
        /// <param name="index">The optional index of the example, the <paramref name="label"/> should be attributed to.</param>
        /// <param name="dictionary">Dictionary used to collect dictifyed features.</param>
        /// <param name="fastDictionary">Dictionary used to collect dictifyed features.</param>
        /// <returns>The serialized Vowpal Wabbit string formatted example.</returns>
        string SerializeToString(TExample example, ILabel label = null, int? index = null, Dictionary<string, string> dictionary = null, Dictionary<object, string> fastDictionary = null);

        /// <summary>
        /// Serializes the given <paramref name="example"/> into a native Vowpal Wabbit example.
        /// </summary>
        /// <param name="example">The example to seralize.</param>
        /// <param name="label">The optional label to serialize.</param>
        /// <param name="index">The optional index of the example, the <paramref name="label"/> should be attributed to.</param>
        /// <returns>A Vowpal Wabbit example ready to be used for prediction and learning.</returns>
        VowpalWabbitExampleCollection Serialize(TExample example, ILabel label = null, int? index = null);
    }
}

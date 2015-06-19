// --------------------------------------------------------------------------------------------------------------------
// <copyright file="IVowpalWabbitVisitor.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace VW.Serializer.Interfaces
{
    /// <summary>
    /// Front-end for serialization.
    /// </summary>
    /// <typeparam name="TResultExample">Type of example produced by front-end.</typeparam>
    public interface IVowpalWabbitVisitor<TResultExample>
    {
        /// <summary>
        /// Invoked for each example. Implementors must dispatch by calling <see cref="IVisitableNamespace{TResultNamespace}.Visit"/>.
        /// </summary>
        /// <param name="label">The Vowpal Wabbit label serialized in the string format.</param>
        /// <param name="namespaces">All discovered namespaces.</param>
        /// <returns>The resulting example representation.</returns>
        TResultExample Visit(string label, IVisitableNamespace[] namespaces);

        /// <summary>
        /// Invoked for each namespace.
        /// </summary>
        /// <typeparam name="T">The element type of the dense namespace.</typeparam>
        /// <param name="namespaceDense">The intermediate namespace representation.</param>
        /// <returns>The resulting namespace representation.</returns>
        void Visit<T>(INamespaceDense<T> namespaceDense);

        /// <summary>
        /// Invoked for each namespace.
        /// </summary>
        /// <param name="namespaceSparse">The intermediate namespace representation.</param>
        /// <returns>The resulting namespace representation.</returns>
        void Visit(INamespaceSparse namespaceSparse);

        /// <summary>
        /// Invoked for each feature. 
        /// </summary>
        /// <typeparam name="T">The original type of the feature.</typeparam>
        /// <param name="feature">The intermediate representation of the feature.</param>
        /// <returns>The resulting feature representation.</returns>
        /// <remarks>The serializer also searches for better matches of Visit implementations (e.g. Visit(IFeature{float} feature).</remarks>
        void Visit<T>(IFeature<T> feature);

        /// <summary>
        /// Invoked for each feature which is additionally flagged by the enumerize option.
        /// </summary>
        /// <typeparam name="T">The original type of the feature.</typeparam>
        /// <param name="feature">The intermediate representation of the feature.</param>
        /// <returns>The resulting feature representation.</returns>
        /// <remarks>The serializer also searches for better matches of Visit implementations (e.g. Visit(IFeature{float} feature).</remarks>
        void VisitEnumerize<T>(IFeature<T> feature);
    }
}

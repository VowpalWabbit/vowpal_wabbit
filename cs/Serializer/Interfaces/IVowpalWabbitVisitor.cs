// --------------------------------------------------------------------------------------------------------------------
// <copyright file="IVowpalWabbitVisitor.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace Microsoft.Research.MachineLearning.Serializer.Interfaces
{
    /// <summary>
    /// Front-end for serialization.
    /// </summary>
    /// <typeparam name="TResultExample">Type of example produced by front-end.</typeparam>
    /// <typeparam name="TResultNamespace">Type of namespace produced by front-end.</typeparam>
    /// <typeparam name="TResultFeature">Type of feature produced by front-end.</typeparam>
    public interface IVowpalWabbitVisitor<TResultExample, TResultNamespace, TResultFeature>
    {
        TResultExample Visit(string label, IVisitableNamespace<TResultNamespace>[] namespaces);

        TResultNamespace Visit<T>(INamespaceDense<T> namespaceDense);

        TResultNamespace Visit(INamespaceSparse<TResultFeature> namespaceSparse);

        TResultFeature Visit<T>(IFeature<T> feature);

        TResultFeature VisitEnumerize<T>(IFeature<T> feature);
    }
}

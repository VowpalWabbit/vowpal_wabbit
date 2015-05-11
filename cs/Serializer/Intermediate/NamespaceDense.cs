using System.Collections.Generic;
using Microsoft.Research.MachineLearning.Serializer.Interfaces;
using System;

namespace Microsoft.Research.MachineLearning.Serializer.Intermediate
{
    public class NamespaceDense<TFeature, TNamespaceResult> : Namespace, INamespaceDense<TFeature>, IVisitableNamespace<TNamespaceResult>
    {
        public Func<TNamespaceResult> Visit { get; set; }

        public IFeature<IEnumerable<TFeature>> DenseFeature { get; set; }
    }
}

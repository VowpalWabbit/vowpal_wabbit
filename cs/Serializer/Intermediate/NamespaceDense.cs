using System.Collections.Generic;
using Microsoft.Research.MachineLearning.Serializer.Interfaces;
using System;

namespace Microsoft.Research.MachineLearning.Serializer.Intermediate
{
    public class NamespaceDense<T, TResult> : Namespace, INamespaceDense<T>, IVisitableNamespace<TResult>
    {
        private Func<INamespaceDense<T>> dispatch;

        public NamespaceDense(Func<INamespaceDense<T>> dispatch)
        {
            this.dispatch = dispatch;
        }

        public IFeature<IEnumerable<T>> DenseFeature { get; set; }

        public TResult Visit()
        {
            return this.dispatch(this);
        }
    }
}

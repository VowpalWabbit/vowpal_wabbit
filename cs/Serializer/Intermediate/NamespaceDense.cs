using System.Collections.Generic;
using VowpalWabbit.Serializer.Interfaces;

namespace VowpalWabbit.Serializer.Intermediate
{
    public class NamespaceDense<T> : Namespace, INamespaceDense<T>
    {
        public IFeature<IEnumerable<T>> DenseFeature { get; set; }
    }
}

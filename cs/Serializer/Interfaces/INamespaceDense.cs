using System.Collections.Generic;

namespace VowpalWabbit.Serializer.Interfaces
{
    public interface INamespaceDense<out T> : INamespace
    {
        IFeature<IEnumerable<T>> DenseFeature { get; }
    }
}

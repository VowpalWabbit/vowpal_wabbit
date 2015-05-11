using System.Collections.Generic;

namespace Microsoft.Research.MachineLearning.Serializer.Interfaces
{
    public interface INamespaceDense<out T> : INamespace
    {
        IFeature<IEnumerable<T>> DenseFeature { get; }
    }
}

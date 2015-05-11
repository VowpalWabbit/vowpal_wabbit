
namespace Microsoft.Research.MachineLearning.Serializer.Interfaces
{
    public interface INamespaceSparse<TResultFeature> : INamespace
    {
        IVisitableFeature<TResultFeature>[] Features { get; }
    }
}

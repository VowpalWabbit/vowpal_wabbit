
namespace VowpalWabbit.Serializer.Interfaces
{
    public interface INamespaceSparse : INamespace
    {
        IFeature[] Features { get; }
    }

}

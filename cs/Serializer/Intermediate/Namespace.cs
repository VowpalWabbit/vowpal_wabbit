using VowpalWabbit.Serializer.Interfaces;

namespace VowpalWabbit.Serializer.Intermediate
{
    public abstract class Namespace : INamespace
    {
        public string Name { get; set; }

        public char FeatureGroup { get; set; }
    }
}

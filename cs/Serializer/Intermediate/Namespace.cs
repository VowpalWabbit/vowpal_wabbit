using Microsoft.Research.MachineLearning.Serializer.Interfaces;

namespace Microsoft.Research.MachineLearning.Serializer.Intermediate
{
    public abstract class Namespace : INamespace
    {
        public string Name { get; set; }

        public char? FeatureGroup { get; set; }
    }
}

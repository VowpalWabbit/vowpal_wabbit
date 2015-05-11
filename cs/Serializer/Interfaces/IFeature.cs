using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Microsoft.Research.MachineLearning.Serializer.Interfaces
{
    public interface IFeature
    {
        string Namespace { get; }

        char? FeatureGroup { get; }

        string Name { get; }

        bool Enumerize { get; }
    }

    public interface IFeature<out T> : IFeature
    {
        T Value { get; }
    }
}

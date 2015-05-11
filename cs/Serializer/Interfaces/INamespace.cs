using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Microsoft.Research.MachineLearning.Serializer.Interfaces
{
    public interface INamespace
    {
        string Name { get; }

        char FeatureGroup { get; }
    }

    public interface IVisitableNamespace<TNamespaceResult> : INamespace
    {
        TNamespaceResult Visit();
    }
}

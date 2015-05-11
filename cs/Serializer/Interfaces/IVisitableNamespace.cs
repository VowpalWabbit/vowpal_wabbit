using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Microsoft.Research.MachineLearning.Serializer.Interfaces
{
    public interface IVisitableNamespace<TNamespaceResult> : INamespace
    {
        Func<TNamespaceResult> Visit { get; }
    }
}

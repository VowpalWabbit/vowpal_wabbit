using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Microsoft.Research.MachineLearning.Serializer.Interfaces
{
    public interface IVisitableFeature<out TResult> : IFeature
    {
        Func<TResult> Visit { get; }
    }
}

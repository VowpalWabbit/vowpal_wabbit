using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Microsoft.Research.MachineLearning
{
    public interface IActionDependentFeatureExample<T>
    {
        IList<T> ActionDependentFeatures { get; }
    }
}

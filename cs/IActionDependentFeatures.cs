using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Microsoft.Research.MachineLearning
{
    public interface IActionDependentFeatures<T>
    {
        ICollection<T> ActionDependentFeatures { get; }
    }
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Microsoft.Research.MachineLearning
{
    public interface IVowpalWabbitExample : IDisposable
    {
        IntPtr Ptr { get; }

        void AddLabel(string label);

        void AddLabel(float label = float.MaxValue, float weight = 1, float initial = 0);
    }
}

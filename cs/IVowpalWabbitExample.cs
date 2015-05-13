using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Microsoft.Research.MachineLearning
{
    public interface IVowpalWabbitExample : IDisposable
    {
        IntPtr Ptr { get; }
    }
}

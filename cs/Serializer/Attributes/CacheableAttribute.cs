using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Microsoft.Research.MachineLearning.Serializer.Attributes
{
    [AttributeUsage(AttributeTargets.Property | AttributeTargets.Class)]
    public class CacheableAttribute : Attribute
    {
        public Type EqualityComparer { get; set; }
    }
}

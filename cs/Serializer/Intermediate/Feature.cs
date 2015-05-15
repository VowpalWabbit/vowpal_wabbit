using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Research.MachineLearning.Serializer.Interfaces;

namespace Microsoft.Research.MachineLearning.Serializer.Intermediate
{
    using VwHandle = IntPtr;

    public class Feature : IFeature
    {
        public string Namespace { get; set; }

        public char? FeatureGroup { get; set; }

        public string Name { get; set; }

        public bool Enumerize { get; set;  }
    }

    public sealed class Feature<T, TResult> : Feature, IFeature<T>, IVisitableFeature<TResult>
    {
        /// <summary>
        /// The actual value
        /// </summary>
        public T Value { get; set; }

        /// <summary>
        /// Compiled func to enable automatic double dispatch.
        /// </summary>
        public Func<TResult> Visit { get; set;  }
    }
}

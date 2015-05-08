using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VowpalWabbit.Serializer.Interfaces
{
    public interface INamespace
    {
        string Name { get; }

        char FeatureGroup { get; }
    }
}

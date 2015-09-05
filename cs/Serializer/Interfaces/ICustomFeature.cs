using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW.Serializer.Intermediate;

namespace VW.Serializer.Interfaces
{
    public interface ICustomFeature
    {
        void Initialize(VowpalWabbit vw, Feature feature);
    }
}

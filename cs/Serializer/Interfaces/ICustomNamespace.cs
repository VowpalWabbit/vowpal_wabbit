using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW.Serializer.Intermediate;

namespace VW.Serializer.Interfaces
{
    public interface ICustomNamespace
    {
        void Initialize(VowpalWabbit vw, Namespace ns);
    }
}

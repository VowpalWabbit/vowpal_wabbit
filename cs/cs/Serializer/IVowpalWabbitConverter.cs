using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW.Serializer.Intermediate;

namespace VW.Serializer
{
    public interface IVowpalWabbitSerializable
    {
        void Marshal(VowpalWabbitMarshalContext ctx, Namespace ns, Feature feature);
    }
}

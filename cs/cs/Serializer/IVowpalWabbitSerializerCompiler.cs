using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VW.Serializer
{
    public interface IVowpalWabbitSerializerCompiler<TExample>
    {
        IVowpalWabbitSerializer<TExample> Create(VowpalWabbit vw);
    }
}

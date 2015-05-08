using VowpalWabbit;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VowpalWabbit.Serializer.Interfaces
{
    public interface IVowpalWabbitSerializer<TContext, TVisitor>
        where TVisitor : IVowpalWabbitVisitor
    {
        void Serialize(TContext context, TVisitor visitor);
    }
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace cs_unittest
{
    public class VowpalWabbitListenerToEvents<T> : VowpalWabbitBaseListener
    {
        public Action<T> Created;
    }
}

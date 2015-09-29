using System;
using VW.Interfaces;

namespace cs_unittest
{
    public class VowpalWabbitListenerToEvents<T> : VowpalWabbitBaseListener
    {
        public Action<T,ILabel> Created;
    }
}

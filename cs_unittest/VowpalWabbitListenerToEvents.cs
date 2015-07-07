using System;

namespace cs_unittest
{
    public class VowpalWabbitListenerToEvents<T> : VowpalWabbitBaseListener
    {
        public Action<T> Created;
    }
}

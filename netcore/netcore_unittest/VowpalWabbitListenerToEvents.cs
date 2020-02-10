using System;
using VW.Labels;

namespace netcore_unittest
{
    public class VowpalWabbitListenerToEvents<T> : VowpalWabbitBaseListener
    {
        public Action<string, T,ILabel> Created;
    }
}

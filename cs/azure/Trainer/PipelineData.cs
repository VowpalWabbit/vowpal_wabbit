using System.Collections.Generic;
using VW;

namespace VowpalWabbit.Azure.Trainer
{
    internal sealed class PipelineData
    {
        internal string JSON { get; set; }

        internal string Offset { get; set; }

        internal string PartitionKey { get; set; }

        internal string EventId { get; set; }

        public VowpalWabbitExampleCollection Example { get; set; }
    }
}
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VowpalWabbit.Azure.Trainer
{
    internal sealed class LatencyOperation : ThrottledOperation<TrainerResult>
    {
        public LatencyOperation() : base(TimeSpan.FromSeconds(1))
        { }

        protected override Task ProcessInternal(TrainerResult value)
        {
            this.telemetry.TrackMetric("End-to-End Latency " + value.PartitionKey, value.Latency.TotalSeconds);
            return Task.FromResult(true);
        }
    }
}

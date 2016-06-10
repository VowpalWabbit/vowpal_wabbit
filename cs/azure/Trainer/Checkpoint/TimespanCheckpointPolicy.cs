using Microsoft.Azure;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VowpalWabbit.Azure.Trainer
{
    public class IntervalCheckpointPolicy : ICheckpointPolicy
    {
        private Stopwatch stopwatch;
        private TimeSpan checkpointInterval;

        public IntervalCheckpointPolicy(TimeSpan checkpointInterval)
        {
            this.stopwatch = Stopwatch.StartNew();
            this.checkpointInterval = checkpointInterval;
        }

        public bool ShouldCheckpointAfterExample(int examples)
        {
            // call checkpoint every 5 minutes, so that worker can resume processing from 5 minutes back if it restarts.
            if (this.stopwatch.Elapsed > checkpointInterval)
            {
                this.stopwatch.Restart();
                return true;
            }

            return false;
        }

        public void Reset()
        {
            this.stopwatch.Restart();
        }

        public override string ToString()
        {
            return $"IntervalCheckpointPolicy: {this.checkpointInterval}";
        }
    }
}

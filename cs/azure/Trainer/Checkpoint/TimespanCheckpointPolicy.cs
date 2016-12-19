// --------------------------------------------------------------------------------------------------------------------
// <copyright file="IntervalCheckpointPolicy.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Diagnostics;

namespace VW.Azure.Trainer.Checkpoint
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

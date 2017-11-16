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
    /// <summary>
    /// Implements a wallclock time based checkpoint policy.
    /// </summary>
    public class IntervalCheckpointPolicy : ICheckpointPolicy
    {
        private Stopwatch stopwatch;
        private TimeSpan checkpointInterval;

        /// <summary>
        /// Initializes a new <see cref="IntervalCheckpointPolicy"/> instance.
        /// </summary>
        public IntervalCheckpointPolicy(TimeSpan checkpointInterval)
        {
            this.stopwatch = Stopwatch.StartNew();
            this.checkpointInterval = checkpointInterval;
        }

        /// <summary>
        /// Return true if the trainer should checkpoint the model, false otherwise.
        /// </summary>
        /// <param name="examples">Number of examples since last checkpoint.</param>
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

        /// <summary>
        /// Reset checkpoint policy state.
        /// </summary>
        public void Reset()
        {
            this.stopwatch.Restart();
        }

        /// <summary>
        /// Serialize to string for logging.
        /// </summary>
        public override string ToString()
        {
            return $"IntervalCheckpointPolicy: {this.checkpointInterval}";
        }
    }
}

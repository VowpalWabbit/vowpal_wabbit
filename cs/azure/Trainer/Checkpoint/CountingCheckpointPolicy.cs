// --------------------------------------------------------------------------------------------------------------------
// <copyright file="CountingCheckpointPolicy.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace VW.Azure.Trainer.Checkpoint
{
    /// <summary>
    /// Implements an example count based checkpoint policy.
    /// </summary>
    public class CountingCheckpointPolicy : ICheckpointPolicy
    {
        private readonly int exampleSyncCount;
        private int exampleCount;

        /// <summary>
        /// Initializes a new <see cref="CountingCheckpointPolicy"/> instance.
        /// </summary>
        public CountingCheckpointPolicy(int exampleSyncCount)
        {
            this.exampleSyncCount = exampleSyncCount;
        }

        /// <summary>
        /// Return true if the trainer should checkpoint the model, false otherwise.
        /// </summary>
        /// <param name="examples">Number of examples since last checkpoint.</param>
        public bool ShouldCheckpointAfterExample(int examples)
        {
            this.exampleCount += examples;

            if (this.exampleCount >= this.exampleSyncCount)
            {
                this.exampleCount %= this.exampleSyncCount;
                return true;
            }

            return false;
        }

        /// <summary>
        /// Reset checkpoint policy state.
        /// </summary>
        public void Reset()
        {
            this.exampleCount = 0;
        }

        /// <summary>
        /// Serialize to string for logging.
        /// </summary>
        public override string ToString()
        {
            return $"CountingCheckpointPolicy: {this.exampleSyncCount}";
        }
    }
}

// --------------------------------------------------------------------------------------------------------------------
// <copyright file="CountingCheckpointPolicy.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace VW.Azure.Trainer.Checkpoint
{
    public class CountingCheckpointPolicy : ICheckpointPolicy
    {
        private readonly int exampleSyncCount;
        private int exampleCount;

        public CountingCheckpointPolicy(int exampleSyncCount)
        {
            this.exampleSyncCount = exampleSyncCount;
        }

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

        public void Reset()
        {
            this.exampleCount = 0;
        }

        public override string ToString()
        {
            return $"CountingCheckpointPolicy: {this.exampleSyncCount}";
        }
    }
}

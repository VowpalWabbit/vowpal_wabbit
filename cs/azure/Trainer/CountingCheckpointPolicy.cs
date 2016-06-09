using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VowpalWabbit.Azure.Trainer
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

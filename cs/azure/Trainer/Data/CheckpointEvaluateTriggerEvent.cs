using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VowpalWabbit.Azure.Trainer.Data
{
    internal sealed class CheckpointEvaluateTriggerEvent
    {
        public override string ToString()
        {
            return "Checkpoint evaluate request";
        }
    }
}

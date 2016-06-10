using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VowpalWabbit.Azure.Trainer.Data
{
    internal sealed class CheckpointData
    {
        internal byte[] Model { get; set; }

        internal byte[] EvalModel { get; set; }

        internal string TrackbackList { get; set; }

        internal bool UpdateClientModel { get; set; }

        internal string State { get; set; }

        internal string Timestamp { get; set; }
    }
}

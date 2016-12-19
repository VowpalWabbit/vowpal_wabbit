// --------------------------------------------------------------------------------------------------------------------
// <copyright file="CheckpointTriggerEvent.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace VW.Azure.Trainer.Data
{
    internal sealed class CheckpointTriggerEvent
    {
        public override string ToString()
        {
            return "Checkpoint request";
        }

        public bool UpdateClientModel { get; set; }
    }
}

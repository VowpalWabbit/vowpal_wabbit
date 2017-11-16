// --------------------------------------------------------------------------------------------------------------------
// <copyright file="CheckpointData.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;

namespace VW.Azure.Trainer.Data
{
    internal sealed class CheckpointData
    {
        internal byte[] Model { get; set; }

        internal byte[] EvalModel { get; set; }

        internal int TrackbackCount { get; set; }

        internal string TrackbackList { get; set; }

        internal bool UpdateClientModel { get; set; }

        internal string State { get; set; }

        internal string Timestamp { get; set; }

        internal DateTime StartDateTime { get; set; }
    }
}

// --------------------------------------------------------------------------------------------------------------------
// <copyright file="TrainerResult.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using VW;
using VW.Labels;

namespace VowpalWabbit.Azure.Trainer.Data
{
    internal sealed class TrainerResult
    {
        public ActionScore[] ProgressivePrediction { get; set; }

        public ContextualBanditLabel Label { get; set; }

        public TimeSpan Latency { get; set; }

        public string PartitionKey { get; set; }
    }
}

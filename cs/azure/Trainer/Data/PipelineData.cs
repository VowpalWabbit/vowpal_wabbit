// --------------------------------------------------------------------------------------------------------------------
// <copyright file="PipelineData.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using VW;

namespace VW.Azure.Trainer.Data
{
    internal sealed class PipelineData
    {
        internal string JSON { get; set; }

        internal string Offset { get; set; }

        internal string PartitionKey { get; set; }

        internal string PartitionId { get; set; }

        internal string EventId { get; set; }

        internal DateTime Timestamp { get; set; }

        internal int[] Actions { get; set; }

        internal float[] Probabilities { get; set; }

        internal float ProbabilityOfDrop { get; set; }

        /// <summary>
        /// 1-based action mapping to tag.
        /// </summary>
        internal Dictionary<int, string> ActionsTags { get; set; } = new Dictionary<int, string>();

        public VowpalWabbitExampleCollection Example { get; set; }
    }
}
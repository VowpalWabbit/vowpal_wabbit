// --------------------------------------------------------------------------------------------------------------------
// <copyright file="TrainerResult.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using VW;
using VW.Labels;

namespace VowpalWabbit.Azure.Trainer.Data
{
    internal sealed class TrainerResult
    {
        internal TrainerResult(ActionScore[] progressivePrediction)
        {
            if (progressivePrediction == null)
                throw new ArgumentNullException(nameof(progressivePrediction));

            this.Ranking = progressivePrediction.Select(a => (int)a.Action).ToArray();
            this.ProbabilitiesOrderedByRanking = progressivePrediction.Select(a => a.Score).ToArray();
            this.Probabilities = new float[ProbabilitiesOrderedByRanking.Length];
            for (int i = 0; i < ProbabilitiesOrderedByRanking.Length; i++)
                this.Probabilities[Ranking[i]] = ProbabilitiesOrderedByRanking[i]; // Ranking is 0-based
        }

        internal ContextualBanditLabel Label { get; set; }

        internal TimeSpan Latency { get; set; }

        internal string PartitionKey { get; set; }

        internal int[] Ranking { get; private set; }

        internal float[] Probabilities { get; private set; }

        internal float[] ProbabilitiesOrderedByRanking { get; private set; }

        internal float ProbabilityOfDrop { get; set; }

        internal Dictionary<int, string> ActionsTags { get; set; }
    }
}

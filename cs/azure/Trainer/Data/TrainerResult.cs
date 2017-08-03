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

namespace VW.Azure.Trainer.Data
{
    internal sealed class TrainerResult
    {
        internal TrainerResult(ActionScore[] progressivePrediction, int[] observedActions, float[] observedProbabilities)
        {
            if (progressivePrediction == null)
                throw new ArgumentNullException(nameof(progressivePrediction));

            if (observedActions == null)
                throw new ArgumentNullException(nameof(observedActions));
            if (observedProbabilities == null)
                throw new ArgumentNullException(nameof(observedProbabilities));

            if (observedActions.Length != observedProbabilities.Length)
                throw new ArgumentException($"Actions (length: {observedActions.Length}) and probabilities (length: {observedProbabilities.Length}) must be of equal length");

            this.ProgressiveRanking = progressivePrediction.Select(a => (int)a.Action).ToArray();
            var probabilitiesOrderedByRanking = progressivePrediction.Select(a => a.Score).ToArray();
            this.ProgressiveProbabilities = new float[probabilitiesOrderedByRanking.Length];
            for (int i = 0; i < probabilitiesOrderedByRanking.Length; i++)
                this.ProgressiveProbabilities[ProgressiveRanking[i]] = probabilitiesOrderedByRanking[i]; // Ranking is 0-based

            this.ObservedRanking = observedActions;
            this.ObservedProbabilities = new float[observedProbabilities.Length];
            for (int i = 0; i < observedActions.Length; i++)
                this.ObservedProbabilities[observedActions[i] - 1] = observedProbabilities[i];
        }

        internal ContextualBanditLabel Label { get; set; }

        internal TimeSpan Latency { get; set; }

        internal string PartitionKey { get; set; }

        internal string PartitionId { get; set; }

        internal int[] ProgressiveRanking { get; private set; }

        internal float[] ProgressiveProbabilities { get; private set; }

        internal int[] ObservedRanking { get; private set; }

        internal float[] ObservedProbabilities { get; private set; }

        internal float ProbabilityOfDrop { get; set; }

        internal Dictionary<int, string> ActionsTags { get; set; }

        internal string EventId { get; set; }

        internal DateTime Timestamp { get; set; }
    }
}

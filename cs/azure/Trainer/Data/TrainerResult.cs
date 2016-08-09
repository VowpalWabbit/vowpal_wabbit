// --------------------------------------------------------------------------------------------------------------------
// <copyright file="TrainerResult.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Linq;
using VW;
using VW.Labels;

namespace VowpalWabbit.Azure.Trainer.Data
{
    internal sealed class TrainerResult
    {
        internal TrainerResult(int[] actions, float[] probabilities)
        {
            if (actions == null)
                throw new ArgumentNullException(nameof(actions));
            if (probabilities == null)
                throw new ArgumentNullException(nameof(probabilities));

            if (actions.Length != probabilities.Length)
                throw new ArgumentException($"Actions (length: {actions.Length}) and probabilities (length: {probabilities.Length}) must be of equal length"); 

            this.Ranking = actions;
            this.ProbabilitiesOrderedByRanking = probabilities;
            this.Probabilities = new float[actions.Length];
            for (int i = 0; i < actions.Length; i++)
                this.Probabilities[actions[i] - 1] = probabilities[i];
        }

        internal ActionScore[] ProgressivePrediction { get; set; }

        internal ContextualBanditLabel Label { get; set; }

        internal TimeSpan Latency { get; set; }

        internal string PartitionKey { get; set; }

        internal int[] Ranking { get; private set; }

        internal float[] Probabilities { get; private set; }

        internal float[] ProbabilitiesOrderedByRanking { get; private set; }

        internal float ProbabilityOfDrop { get; set; }
    }
}

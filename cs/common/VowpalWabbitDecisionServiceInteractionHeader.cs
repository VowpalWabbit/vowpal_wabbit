// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitDecisionServiceInteraction.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace VW
{
    /// <summary>
    /// Decision Service interaction header information.
    /// </summary>
    public sealed class VowpalWabbitDecisionServiceInteractionHeader
    {
        /// <summary>
        /// EventId extracted from Decision Service Interaction JSON data.
        /// </summary>
        public string EventId { get; set; }

        /// <summary>
        /// Actions extracted from Decision Service Interaction JSON data.
        /// </summary>
        public int[] Actions { get; set; }

        /// <summary>
        /// Probabilities extracted from Decision Service Interaction JSON data.
        /// </summary>
        public float[] Probabilities { get; set; }

        /// <summary>
        /// Probability of drop extracted from Decision Service Interaction JSON data.
        /// </summary>
        public float ProbabilityOfDrop { get; set; }
    }
}

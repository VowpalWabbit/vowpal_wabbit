// --------------------------------------------------------------------------------------------------------------------
// <copyright file="ICheckpointPolicy.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace VW.Azure.Trainer.Checkpoint
{
    /// <summary>
    /// Interface for model checkpoint policies.
    /// </summary>
    public interface ICheckpointPolicy
    {
        /// <summary>
        /// Return true if the trainer should checkpoint the model, false otherwise.
        /// </summary>
        /// <param name="examples">Number of examples since last checkpoint.</param>
        bool ShouldCheckpointAfterExample(int examples);

        /// <summary>
        /// Reset checkpoint policy state.
        /// </summary>
        void Reset();
    }
}

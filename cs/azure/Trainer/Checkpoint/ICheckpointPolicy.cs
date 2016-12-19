// --------------------------------------------------------------------------------------------------------------------
// <copyright file="ICheckpointPolicy.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace VW.Azure.Trainer.Checkpoint
{
    public interface ICheckpointPolicy
    {
        bool ShouldCheckpointAfterExample(int examples);

        void Reset();
    }
}

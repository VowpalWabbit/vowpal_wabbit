// --------------------------------------------------------------------------------------------------------------------
// <copyright file="OnlineTrainerSettingsInternal.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using VowpalWabbit.Azure.Trainer.Checkpoint;

namespace VowpalWabbit.Azure.Trainer
{
    public class OnlineTrainerSettingsInternal
    {
        public string StorageContainerName { get; private set; } = "onlinetrainer";

        public OnlineTrainerSettings Metadata { get; set;  }

        /// <summary>
        /// The initial model the training run started from.
        /// </summary>
        public string InitialVowpalWabbitModel { get; set; }

        public string StorageConnectionString { get; set; }

        public string JoinedEventHubConnectionString { get; set; }

        public string EvalEventHubConnectionString { get; set; }

        /// <summary>
        /// Checkpoint policy.
        /// </summary>
        public ICheckpointPolicy CheckpointPolicy { get; set; }

        public bool EnableExampleTracing { get; set; }

        internal bool ForceFreshStart { get; set; }
    }
}

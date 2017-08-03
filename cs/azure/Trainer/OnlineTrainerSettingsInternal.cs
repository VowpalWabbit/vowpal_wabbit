// --------------------------------------------------------------------------------------------------------------------
// <copyright file="OnlineTrainerSettingsInternal.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using Microsoft.ServiceBus.Messaging;
using System;
using VW.Azure.Trainer.Checkpoint;

namespace VW.Azure.Trainer
{
    /// <summary>
    /// The internal trainer settings.
    /// </summary>
    public class OnlineTrainerSettingsInternal
    {
        /// <summary>
        /// The Azure storage container name used for model and state history.
        /// </summary>
        public string StorageContainerName { get; private set; } = "onlinetrainer";

        /// <summary>
        /// External supplied meta data.
        /// </summary>
        public OnlineTrainerSettings Metadata { get; set;  }

        /// <summary>
        /// The initial model the training run started from.
        /// </summary>
        public string InitialVowpalWabbitModel { get; set; }

        /// <summary>
        /// Azure storage connection string used for checkpointing.
        /// </summary>
        public string StorageConnectionString { get; set; }

        /// <summary>
        /// Input data Azure EventHub connection string.
        /// </summary>
        public string JoinedEventHubConnectionString { get; set; }

        /// <summary>
        /// Consumer group used for joined events.
        /// </summary>
        public string JoinedEventHubConsumerGroup { get; set; } = EventHubConsumerGroup.DefaultGroupName;

        /// <summary>
        /// Evaluation output Azure Eventhub connection string.
        /// </summary>
        public string EvalEventHubConnectionString { get; set; }

        /// <summary>
        /// Checkpoint policy.
        /// </summary>
        public ICheckpointPolicy CheckpointPolicy { get; set; }

        /// <summary>
        /// True if examples should be traced.
        /// </summary>
        public bool EnableExampleTracing { get; set; }

        /// <summary>
        /// Null will let the trainer read events earliest available timestamps in event hub input;
        /// Any other valid DateTime will let the trainer read events from that point in time.
        /// </summary>
        public DateTime? EventHubStartDateTimeUtc { get; set; }

        /// <summary>
        /// True if a fresh start was forced.
        /// </summary>
        internal bool ForceFreshStart { get; set; }
    }
}

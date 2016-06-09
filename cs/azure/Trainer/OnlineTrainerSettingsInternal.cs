namespace VowpalWabbit.Azure.Trainer
{
    public class OnlineTrainerSettingsInternal
    {
        public string StorageContainerName { get; private set; } = "onlinetrainer";

        public string SettingsUrl { get; set; }

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

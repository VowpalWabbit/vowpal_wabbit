using Microsoft.ApplicationInsights;
using Microsoft.ApplicationInsights.DataContracts;
using Microsoft.ServiceBus;
using Microsoft.ServiceBus.Messaging;
using System;
using System.Collections.Generic;
using System.Reactive.Linq;
using System.Threading;
using System.Threading.Tasks;
using System.Threading.Tasks.Dataflow;
using VW.Azure.Trainer.Data;
using VW.Azure.Trainer.Operations;
using VW.Serializer;

namespace VW.Azure.Trainer
{
    /// <summary>
    /// Azure online trainer.
    /// </summary>
    public sealed class LearnEventProcessorHost : IDisposable
    {
        private readonly TelemetryClient telemetry;

        private readonly object managementLock = new object();
        private TrainEventProcessorFactory trainProcessorFactory;
        private EventProcessorHost eventProcessorHost;
        private Learner trainer;
        private PerformanceCounters perfCounters;
        private SafeTimer perfUpdater;
        private DateTime? eventHubStartDateTimeUtc;

        /// <summary>
        /// Initializes a new <see cref="LearnEventProcessorHost"/> instance.
        /// </summary>
        public LearnEventProcessorHost()
        {
            this.telemetry = new TelemetryClient();
            
            // by default read from the beginning of Event Hubs event stream.
            this.eventHubStartDateTimeUtc = null;
        }

        /// <summary>
        /// Performance countners populated by online trainer.
        /// </summary>
        public PerformanceCounters PerformanceCounters { get { return this.perfCounters; } }

        /// <summary>
        /// Timestamp when the trainer was last started.
        /// </summary>
        public DateTime LastStartDateTimeUtc { get; private set; }

        internal object InitialOffsetProvider(string partition)
        {
            string offset;
            if (this.trainer.State.Partitions.TryGetValue(partition, out offset))
                return offset;

            // either DateTime.UtcNow on reset or null if start the first time
            return this.eventHubStartDateTimeUtc;
        }

        /// <summary>
        /// Starts the trainer with given parameters.
        /// </summary>
        public async Task StartAsync(OnlineTrainerSettingsInternal settings)
        {
            await this.SafeExecute(async () => await this.StartInternalAsync(settings));
        }

        /// <summary>
        /// Stops the trainer.
        /// </summary>
        /// <returns></returns>
        public async Task StopAsync()
        {
            await this.SafeExecute(this.StopInternalAsync);
        }

        /// <summary>
        /// Restarts the trainer.
        /// </summary>
        public async Task Restart(OnlineTrainerSettingsInternal settings)
        {
            await this.SafeExecute(async () => await this.RestartInternalAsync(settings));
        }

        /// <summary>
        /// Resets the trainers.
        /// </summary>
        public async Task ResetModelAsync(OnlineTrainerState state = null, byte[] model = null)
        {
            await this.SafeExecute(async () => await this.ResetInternalAsync(state, model));
        }

        /// <summary>
        /// Forces model checkpointing.
        /// </summary>
        public async Task CheckpointAsync()
        {
            await this.SafeExecute(async () => await this.trainProcessorFactory.LearnBlock.SendAsync(new CheckpointTriggerEvent()));
        }

        private Task SafeExecute(Func<Task> action)
        {
            try
            {
                // need to do a lock as child tasks are interleaving
                lock (this.managementLock)
                {
                    action().Wait(TimeSpan.FromMinutes(3));
                }
            }
            catch (AggregateException ex)
            {
                foreach (var innerEx in ex.Flatten().InnerExceptions)
                    this.telemetry.TrackException(innerEx);
                throw ex;
            }
            catch (Exception ex)
            {
                this.telemetry.TrackException(ex);
                throw ex;
            }

            return Task.FromResult(true);
        }
        
        private async Task ResetInternalAsync(OnlineTrainerState state = null, byte[] model = null)
        {
            if (this.trainer == null)
            {
                this.telemetry.TrackTrace("Online Trainer resetting skipped as trainer hasn't started yet.", SeverityLevel.Information);
                return;
            }

            var msg = "Online Trainer resetting";
            bool updateClientModel = false;
            if (state != null)
            {
                msg += "; state supplied";
                updateClientModel = true;
            }
            if (model != null)
            {
                msg += $"; model of size {model.Length} supplied.";
                updateClientModel = true;
            }

            this.telemetry.TrackTrace(msg, SeverityLevel.Information);

            var settings = this.trainer.Settings;

            await this.StopInternalAsync();

            settings.ForceFreshStart = true;
            settings.CheckpointPolicy.Reset();

            await this.StartInternalAsync(settings, state, model);

            // make sure we store this fresh model, in case we die we don't loose the reset
            await this.trainProcessorFactory.LearnBlock.SendAsync(new CheckpointTriggerEvent { UpdateClientModel = updateClientModel });

            if (!updateClientModel)
            {
                // delete the currently deployed model, so the clients don't use the hold one
                var latestModel = await this.trainer.GetLatestModelBlob();
                this.telemetry.TrackTrace($"Resetting client visible model: {latestModel.Uri}", SeverityLevel.Information);
                await latestModel.UploadFromByteArrayAsync(new byte[0], 0, 0);
            }
        }

        private async Task RestartInternalAsync(OnlineTrainerSettingsInternal settings)
        {
            this.telemetry.TrackTrace("Online Trainer restarting", SeverityLevel.Information);

            await this.StopInternalAsync();

            await this.StartInternalAsync(settings);
        }

        private async Task StartInternalAsync(OnlineTrainerSettingsInternal settings, OnlineTrainerState state = null, byte[] model = null)
        {
            this.LastStartDateTimeUtc = DateTime.UtcNow;
            this.perfCounters = new PerformanceCounters(settings.Metadata.ApplicationID);

            // setup trainer
            this.trainer = new Learner(settings, this.DelayedExampleCallback, this.perfCounters);

            if (settings.ForceFreshStart || model != null)
                this.trainer.FreshStart(state, model);
            else
                await this.trainer.FindAndResumeFromState();

            // setup factory
            this.trainProcessorFactory = new TrainEventProcessorFactory(settings, this.trainer, this.perfCounters);

            // setup host
            var serviceBusConnectionStringBuilder = new ServiceBusConnectionStringBuilder(settings.JoinedEventHubConnectionString);
            var joinedEventhubName = serviceBusConnectionStringBuilder.EntityPath;
            serviceBusConnectionStringBuilder.EntityPath = string.Empty;

            this.eventProcessorHost = new EventProcessorHost(settings.Metadata.ApplicationID, joinedEventhubName,
                settings.JoinedEventHubConsumerGroup, serviceBusConnectionStringBuilder.ToString(), settings.StorageConnectionString);

            // used by this.InitialOffsetProvider if no checkpointed state is found
            this.eventHubStartDateTimeUtc = settings.EventHubStartDateTimeUtc;

            await this.eventProcessorHost.RegisterEventProcessorFactoryAsync(
                this.trainProcessorFactory,
                new EventProcessorOptions { InitialOffsetProvider = this.InitialOffsetProvider });

            // don't perform too often
            this.perfUpdater = new SafeTimer(
                TimeSpan.FromMilliseconds(500),
                this.UpdatePerformanceCounters);

            var vwArgs = this.trainer.VowpalWabbit.Arguments;

            this.telemetry.TrackTrace(
                "OnlineTrainer started",
                SeverityLevel.Information,
                new Dictionary<string, string>
                {
                    { "CheckpointPolicy", settings.CheckpointPolicy.ToString() },
                    { "VowpalWabbit", settings.Metadata.TrainArguments },
                    { "ExampleTracing", settings.EnableExampleTracing.ToString() },
                    { "LearningRate", vwArgs.LearningRate.ToString() },
                    { "PowerT", vwArgs.PowerT.ToString() }
                });
        }

        private void UpdatePerformanceCounters()
        {
            lock (this.managementLock)
            {
                // make sure this is thread safe w.r.t reset/start/stop/...
                try
                {
                    if (this.trainer != null && this.trainProcessorFactory != null)
                    {
                        this.trainer.UpdatePerformanceCounters();
                        this.trainProcessorFactory.UpdatePerformanceCounters();
                    }
                }
                catch (Exception ex)
                {
                    this.telemetry.TrackException(ex);
                }
            }
        }

        private async Task StopInternalAsync()
        {
            this.telemetry.TrackTrace("OnlineTrainer stopping", SeverityLevel.Verbose);

            if (this.perfUpdater != null)
            {
                this.perfUpdater.Stop(TimeSpan.FromMinutes(1));
                this.perfUpdater = null;
            }

            if (this.eventProcessorHost != null)
            {
                try
                {
                    await this.eventProcessorHost.UnregisterEventProcessorAsync();
                }
                catch (Exception ex)
                {
                    this.telemetry.TrackException(ex);
                }

                this.eventProcessorHost = null;
            }

            if (this.trainProcessorFactory != null)
            {
                // flushes the pipeline
                this.trainProcessorFactory.Dispose();
                this.trainProcessorFactory = null;
            }

            if (this.trainer != null)
            {
                this.trainer.Dispose();
                this.trainer = null;
            }

            if (this.perfCounters != null)
            {
                this.perfCounters.Dispose();
                this.perfCounters = null;
            }

            this.telemetry.TrackTrace("OnlineTrainer stopped", SeverityLevel.Verbose);
        }

        private void DelayedExampleCallback(VowpalWabbitJsonSerializer serializer)
        {
            try
            {
                this.perfCounters.Feature_Requests_Pending.IncrementBy(-1);

                var data = (PipelineData)serializer.UserContext;
                data.Example = serializer.CreateExamples();


                // fire and forget
                // must not block to avoid dead lock
                this.trainProcessorFactory.LearnBlock
                    .SendAsync(data)
                    .ContinueWith(async ret =>
                    {
                        if (!await ret)
                        {
                            this.telemetry.TrackTrace("Unable to enqueue delayed examples", SeverityLevel.Error);

                            // since we couldn't enqueue, need to dispose here
                            data.Example.Dispose();
                        }
                    });
            }
            catch (Exception e)
            {
                this.telemetry.TrackException(e);
            }
            finally
            {
                serializer.Dispose();
            }
        }

        /// <summary>
        /// Dispose the trainer.
        /// </summary>
        public void Dispose()
        {
            try
            {
                this.StopAsync().Wait(TimeSpan.FromMinutes(1));
            }
            catch (Exception ex)
            {
                this.telemetry.TrackException(ex);
            }
        }
    }
}

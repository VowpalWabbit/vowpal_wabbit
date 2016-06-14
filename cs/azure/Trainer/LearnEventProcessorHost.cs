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
using VW.Serializer;

namespace VowpalWabbit.Azure.Trainer
{
    public sealed class LearnEventProcessorHost : IDisposable
    {
        private readonly TelemetryClient telemetry;

        /// <summary>
        /// During start/stop/restart operations the objects below gets reset.
        /// Make sure this is done in a consistent manner.
        /// </summary>
        private readonly ConcurrentExclusiveSchedulerPair exclusiveScheduler;
        private readonly TaskFactory exclusiveTaskFactory;
        private TrainEventProcessorFactory trainProcessorFactory;
        private EventProcessorHost eventProcessorHost;
        private Learner trainer;
        private PerformanceCounters perfCounters;
        private SafeTimer perfUpdater;

        public LearnEventProcessorHost()
        {
            this.telemetry = new TelemetryClient();
            this.exclusiveScheduler = new ConcurrentExclusiveSchedulerPair(TaskScheduler.Default, 1);
            this.exclusiveTaskFactory = new TaskFactory(exclusiveScheduler.ExclusiveScheduler);
        }

        internal object InitialOffsetProvider(string partition)
        {
            string offset;
            if (this.trainer.State.Partitions.TryGetValue(partition, out offset))
                return offset;

            return DateTime.UtcNow;
        }

        public async Task StartAsync(OnlineTrainerSettingsInternal settings)
        {
            await this.SafeExecute(async () => await this.StartInternalAsync(settings));
        }

        public async Task StopAsync()
        {
            await this.SafeExecute(this.StopInternalAsync);
        }

        public async Task Restart(OnlineTrainerSettingsInternal settings)
        {
            await this.SafeExecute(async () => await this.RestartInternalAsync(settings));
        }

        public async Task ResetModelAsync(OnlineTrainerState state = null)
        {
            await this.SafeExecute(async () => await this.ResetInternalAsync(state));
        }

        public async Task CheckpointAsync()
        {
            await this.SafeExecute(async () => await this.trainProcessorFactory.LearnBlock.SendAsync(new CheckpointEvent()));
        }

        private Task SafeExecute(Func<Task> action)
        {
            try
            {
                return exclusiveTaskFactory.StartNew(action).Unwrap();
            }
            catch (Exception ex)
            {
                this.telemetry.TrackException(ex);
                throw ex;
            }
        }
        
        private async Task ResetInternalAsync(OnlineTrainerState state = null)
        {
            this.telemetry.TrackTrace("Online Trainer resetting", SeverityLevel.Information);

            var settings = this.trainer.Settings;

            await this.StopInternalAsync();

            settings.ForceFreshStart = true;
            settings.CheckpointPolicy.Reset();

            await this.StartInternalAsync(settings, state);

            // make sure we store this fresh model, in case we die we don't loose the reset
            await this.trainProcessorFactory.LearnBlock.SendAsync(new CheckpointEvent());

            // delete the currently deployed model, so the clients don't use the hold one
            var latestModel = await this.trainer.GetLatestModelBlob();
            this.telemetry.TrackTrace($"Resetting client visible model: {latestModel.Uri}", SeverityLevel.Information);
            await latestModel.UploadFromByteArrayAsync(new byte[0], 0, 0);
        }

        private async Task RestartInternalAsync(OnlineTrainerSettingsInternal settings)
        {
            this.telemetry.TrackTrace("Online Trainer restarting", SeverityLevel.Information);

            await this.StopInternalAsync();

            await this.StartInternalAsync(settings);
        }

        private async Task StartInternalAsync(OnlineTrainerSettingsInternal settings, OnlineTrainerState state = null)
        {
            this.perfCounters = new PerformanceCounters(settings.Metadata.ApplicationID);

            // setup trainer
            this.trainer = new Learner(settings, this.DelayedExampleCallback, this.perfCounters);

            if (settings.ForceFreshStart)
                this.trainer.FreshStart(state);
            else
                await this.trainer.FindAndResumeFromState();

            // setup factory
            this.trainProcessorFactory = new TrainEventProcessorFactory(settings, this.trainer, this.perfCounters);

            // setup host
            var serviceBusConnectionStringBuilder = new ServiceBusConnectionStringBuilder(settings.JoinedEventHubConnectionString);
            var joinedEventhubName = serviceBusConnectionStringBuilder.EntityPath;
            serviceBusConnectionStringBuilder.EntityPath = string.Empty;

            this.eventProcessorHost = new EventProcessorHost(settings.Metadata.ApplicationID, joinedEventhubName,
                EventHubConsumerGroup.DefaultGroupName, serviceBusConnectionStringBuilder.ToString(), settings.StorageConnectionString);

            await this.eventProcessorHost.RegisterEventProcessorFactoryAsync(
                this.trainProcessorFactory,
                new EventProcessorOptions { InitialOffsetProvider = this.InitialOffsetProvider });

            // don't perform too often
            this.perfUpdater = new SafeTimer(
                TimeSpan.FromMilliseconds(500),
                this.UpdatePerformanceCounters);

            this.telemetry.TrackTrace(
                "OnlineTrainer started",
                SeverityLevel.Information,
                new Dictionary<string, string>
                {
                { "CheckpointPolicy", settings.CheckpointPolicy.ToString() },
                { "VowpalWabbit", settings.Metadata.TrainArguments },
                { "ExampleTracing", settings.EnableExampleTracing.ToString() }
                });
        }

        private void UpdatePerformanceCounters()
        {
            try
            {
                this.trainer.UpdatePerformanceCounters();
                this.trainProcessorFactory.UpdatePerformanceCounters();
            }
            catch (Exception ex)
            {
                this.telemetry.TrackException(ex);
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

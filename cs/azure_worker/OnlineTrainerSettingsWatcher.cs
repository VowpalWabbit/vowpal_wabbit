// --------------------------------------------------------------------------------------------------------------------
// <copyright file="OnlineTrainerSettingsWatcher.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using Microsoft.ApplicationInsights;
using Microsoft.ApplicationInsights.DataContracts;
using Microsoft.Azure;
using Microsoft.ServiceBus;
using Microsoft.WindowsAzure.ServiceRuntime;
using Microsoft.WindowsAzure.Storage;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using VW.Azure.Trainer;
using VW.Azure.Trainer.Checkpoint;

namespace VW.Azure.Worker
{
    internal sealed class OnlineTrainerSettingsWatcher : IDisposable
    {
        private readonly TelemetryClient telemetry;
        private readonly LearnEventProcessorHost trainProcessorHost;

        private OnlineTrainerSettingsDownloader settingsDownloader;
        private OnlineTrainerSettings metaData;

        internal OnlineTrainerSettingsWatcher(LearnEventProcessorHost trainProcessorHost)
        {
            this.telemetry = new TelemetryClient();
            this.trainProcessorHost = trainProcessorHost;

            RoleEnvironment.Changed += RoleEnvironment_Changed;

            this.settingsDownloader = new OnlineTrainerSettingsDownloader(TimeSpan.FromSeconds(5));
            this.settingsDownloader.Downloaded += AzureSettingsBlobDownloader_Downloaded;
            this.settingsDownloader.Failed += AzureSettingsBlobDownloader_Failed;
        }

        public void RestartTrainProcessorHost()
        {
            if (this.metaData == null)
                return;

            var settings = new OnlineTrainerSettingsInternal
            {
                StorageConnectionString = CloudConfigurationManager.GetSetting("StorageConnectionString"),
                JoinedEventHubConnectionString = CloudConfigurationManager.GetSetting("JoinedEventHubConnectionString"),
                EvalEventHubConnectionString = CloudConfigurationManager.GetSetting("EvalEventHubConnectionString"),
                Metadata = this.metaData,
                CheckpointPolicy = ParseCheckpointPolicy(),
                // make sure we ignore previous events
                EventHubStartDateTimeUtc = DateTime.UtcNow 
            };

            var joinedEventHubConsumerGroup = CloudConfigurationManager.GetSetting("JoinedEventHubConsumerGroup");
            if (!string.IsNullOrEmpty(joinedEventHubConsumerGroup))
                settings.JoinedEventHubConsumerGroup = joinedEventHubConsumerGroup;

            bool enableExampleTracing;
            if (bool.TryParse(CloudConfigurationManager.GetSetting("EnableExampleTracing"), out enableExampleTracing))
                settings.EnableExampleTracing = enableExampleTracing;

            ServiceBusConnectionStringBuilder serviceBusConnectionStringBuilder;
            try
            {
                serviceBusConnectionStringBuilder = new ServiceBusConnectionStringBuilder(settings.JoinedEventHubConnectionString);
            }
            catch (Exception e)
            {
                throw new InvalidDataException($"Invalid JoinedEventHubConnectionString '{settings.JoinedEventHubConnectionString}' found in settings: {e.Message}");
            }

            try
            {
                serviceBusConnectionStringBuilder = new ServiceBusConnectionStringBuilder(settings.EvalEventHubConnectionString);
            }
            catch (Exception e)
            {
                throw new InvalidDataException($"Invalid EventHubEvalConnectionString '{settings.EvalEventHubConnectionString}' found in settings: {e.Message}");
            }

            if (string.IsNullOrEmpty(serviceBusConnectionStringBuilder.EntityPath))
                throw new InvalidDataException($"Invalid EventHubEvalConnectionString '{settings.EvalEventHubConnectionString}' found in settings: EntityPath missing");

            CloudStorageAccount cloudStorageAccount;
            if (!CloudStorageAccount.TryParse(settings.StorageConnectionString, out cloudStorageAccount))
                throw new InvalidDataException($"Invalid StorageConnectionString '{settings.StorageConnectionString}' found in settings");

            // fire and forget
            var task = this.trainProcessorHost.Restart(settings);
        }

        private void AzureSettingsBlobDownloader_Failed(object sender, Exception e)
        {
            this.telemetry.TrackException(e);
        }

        private void AzureSettingsBlobDownloader_Downloaded(object sender, byte[] data)
        {
            try
            {
                var json = Encoding.UTF8.GetString(data);
                this.metaData = JsonConvert.DeserializeObject<OnlineTrainerSettings>(json);

                this.telemetry.TrackTrace(
                    "Metadata update. Trigger restart",
                    SeverityLevel.Information,
                    new Dictionary<string, string> { { "settings", json } });

                this.RestartTrainProcessorHost();
            }
            catch (Exception ex)
            {
                this.telemetry.TrackException(ex);
            }
        }

        private void RoleEnvironment_Changed(object sender, RoleEnvironmentChangedEventArgs e)
        {
            try
            {
                var changes = e.Changes.OfType<RoleEnvironmentConfigurationSettingChange>()
                    .Select(c => c.ConfigurationSettingName)
                    .ToList();

                if (changes.Count > 0)
                {
                    this.telemetry.TrackTrace(
                        "Configuration changes. Trigger restart",
                        SeverityLevel.Information,
                        changes.ToDictionary(name => name, name => CloudConfigurationManager.GetSetting(name)));

                    // fire and forget
                    this.RestartTrainProcessorHost();
                }
            }
            catch (Exception ex)
            {
                this.telemetry.TrackException(ex);
            }
        }

        private ICheckpointPolicy ParseCheckpointPolicy()
        {
            var checkpointString = CloudConfigurationManager.GetSetting("CheckpointIntervalOrCount");
            if (!string.IsNullOrWhiteSpace(checkpointString))
            {
                if (checkpointString.Contains(":"))
                {
                    TimeSpan interval;
                    if (TimeSpan.TryParse(checkpointString, CultureInfo.InvariantCulture, out interval))
                        return new IntervalCheckpointPolicy(interval);
                }
                else
                {
                    int syncCount;
                    if (int.TryParse(checkpointString, NumberStyles.Integer, CultureInfo.InvariantCulture, out syncCount))
                        return new CountingCheckpointPolicy(syncCount);
                }
            }

            this.telemetry.TrackTrace("No valid checkpoint policy found. Defaulting to 5 minute wallclock checkpointing.");

            return new IntervalCheckpointPolicy(TimeSpan.FromMinutes(5));
        }

        public void Dispose()
        {
            RoleEnvironment.Changed -= RoleEnvironment_Changed;

            if (this.settingsDownloader != null)
            {
                this.settingsDownloader.Dispose();
                this.settingsDownloader = null;
            }
        }
    }
}

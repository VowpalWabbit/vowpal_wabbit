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
using VowpalWabbit.Azure.Trainer;

namespace VowpalWabbit.Azure.Worker
{
    internal sealed class OnlineTrainerSettingsWatcher : IDisposable
    {
        private readonly TelemetryClient telemetry;
        private readonly LearnEventProcessorHost trainProcessorHost;

        private AzureBlobBackgroundDownloader azureSettingsBlobDownloader;
        private OnlineTrainerSettings metaData;

        internal OnlineTrainerSettingsWatcher(LearnEventProcessorHost trainProcessorHost)
        {
            this.telemetry = new TelemetryClient();
            this.trainProcessorHost = trainProcessorHost;

            RoleEnvironment.Changed += RoleEnvironment_Changed;

            this.azureSettingsBlobDownloader = new AzureBlobBackgroundDownloader(
                GetSettingsUrl(),
                TimeSpan.FromSeconds(5));

            this.azureSettingsBlobDownloader.Downloaded += AzureSettingsBlobDownloader_Downloaded;
            this.azureSettingsBlobDownloader.Failed += AzureSettingsBlobDownloader_Failed;
        }

        private static string GetSettingsUrl()
        {
            var storageAccount = CloudStorageAccount.Parse(CloudConfigurationManager.GetSetting("StorageConnectionString"));

            return $"{storageAccount.BlobStorageUri.PrimaryUri}{OnlineTrainerSettings.SettingsContainerName}/{OnlineTrainerSettings.LatestClientSettingsBlobName}";
        }

        private static OnlineTrainerSettings DownloadAndValidate()
        {
            var uri = GetSettingsUrl();
            string jsonMetadata = "";
            try
            {
                using (var wc = new WebClient())
                {
                    jsonMetadata = wc.DownloadString(uri);
                    return JsonConvert.DeserializeObject<OnlineTrainerSettings>(jsonMetadata);
                }
            }
            catch (Exception ex)
            {
                throw new InvalidDataException("Unable to download metadata from specified blob uri '" + uri + "', JSON: " + jsonMetadata, ex);
            }
        }
        
        public void RestartTrainProcessorHost()
        {
            if (this.metaData == null)
                return;

            var settings = new OnlineTrainerSettingsInternal
            {
                SettingsUrl = GetSettingsUrl(),
                StorageConnectionString = CloudConfigurationManager.GetSetting("StorageConnectionString"),
                JoinedEventHubConnectionString = CloudConfigurationManager.GetSetting("JoinedEventHubConnectionString"),
                EvalEventHubConnectionString = CloudConfigurationManager.GetSetting("EvalEventHubConnectionString"),
                Metadata = this.metaData,
                CheckpointPolicy = ParseCheckpointPolicy()
            };

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
            this.trainProcessorHost.Restart(settings);
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
            var checkpointString = CloudConfigurationManager.GetSetting("CheckpointInterval");
            TimeSpan interval;
            if (!string.IsNullOrWhiteSpace(checkpointString) &&
                TimeSpan.TryParse(checkpointString, CultureInfo.InvariantCulture, out interval))
            {
                return new IntervalCheckpointPolicy(interval);
            }

            checkpointString = CloudConfigurationManager.GetSetting("CheckpointCount");
            int syncCount;
            if (!string.IsNullOrWhiteSpace(checkpointString) &&
                int.TryParse(checkpointString, NumberStyles.Integer, CultureInfo.InvariantCulture, out syncCount))
            {
                return new CountingCheckpointPolicy(syncCount);
            }

            this.telemetry.TrackTrace("No valid checkpoint policy found. Defaulting to 5 minute wallclock checkpointing.");

            return new IntervalCheckpointPolicy(TimeSpan.FromMinutes(5));
        }

        public void Dispose()
        {
            RoleEnvironment.Changed -= RoleEnvironment_Changed;

            if (this.azureSettingsBlobDownloader != null)
            {
                this.azureSettingsBlobDownloader.Dispose();
                this.azureSettingsBlobDownloader = null;
            }
        }
    }
}

using Microsoft.ApplicationInsights;
using Microsoft.WindowsAzure.Storage;
using Microsoft.WindowsAzure.Storage.Blob;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Reactive.Linq;
using System.Threading.Tasks;
using VW;
using VW.Serializer;

namespace VW.Azure.Trainer
{
    internal partial class Learner : IDisposable
    {
        internal const string StateBlobName = "state.json";

        private readonly TelemetryClient telemetry;

        private CloudBlobClient blobClient;

        private VW.VowpalWabbit vw;

        private VowpalWabbitJsonReferenceResolver referenceResolver;

        private List<string> trackbackList;

        private OnlineTrainerSettingsInternal settings;

        private OnlineTrainerState state;

        private readonly Action<VowpalWabbitJsonSerializer> delayedExampleCallback;

        private DateTime startDateTime;

        private readonly PerformanceCounters perfCounters;

        //private VowpalWabbitThreadedLearning vwAllReduce;

        internal Learner(OnlineTrainerSettingsInternal settings, Action<VowpalWabbitJsonSerializer> delayedExampleCallback, PerformanceCounters perfCounters)
        {
            this.telemetry = new TelemetryClient();

            this.settings = settings;
            this.delayedExampleCallback = delayedExampleCallback;
            this.perfCounters = perfCounters;

            this.trackbackList = new List<string>();
            this.blobClient = CloudStorageAccount.Parse(settings.StorageConnectionString).CreateCloudBlobClient();
        }

        internal void UpdatePerformanceCounters()
        {
            if (this.referenceResolver != null)
            {
                // don't do this too often as it grabs a lock
                var stats = this.referenceResolver.Statistics;
                this.perfCounters.Features_Cached.RawValue = stats.ItemCount;
                this.perfCounters.Feature_Requests_Pending.RawValue = stats.NumberOfOpenRequests;
            }
        }

        internal OnlineTrainerState State
        {
            get { return this.state; }
        }

        internal OnlineTrainerSettingsInternal Settings
        {
            get { return this.settings; }
        }

        /// <summary>
        /// create light-weight VW instance for example deserialization
        /// need thread-safe example pool wrapper as the examples are allocated and disposed on different threads
        /// </summary>
        internal VW.VowpalWabbit VowpalWabbit
        {
            get { return this.vw; }
        }

        internal VowpalWabbitJsonReferenceResolver ReferenceResolver
        {
            get { return this.referenceResolver; }
        }

        internal async Task<CloudBlockBlob> GetLatestModelBlob()
        {
            var latestModelContainerName = OnlineTrainerSettings.ModelContainerName;
            var mwtModelContainer = this.blobClient.GetContainerReference(latestModelContainerName);
            await mwtModelContainer.CreateIfNotExistsAsync();

            return mwtModelContainer.GetBlockBlobReference(OnlineTrainerSettings.LatestModelBlobName);
        }

        /// <summary>
        /// Performs application-defined tasks associated with freeing, releasing, or resetting unmanaged resources.
        /// </summary>
        public void Dispose()
        {
            //if (this.vwAllReduce != null)
            //{
            //    this.vwAllReduce.Dispose();
            //    this.vwAllReduce = null;
            //}

            if (this.vw != null)
            {
                this.vw.Dispose();
                this.vw = null;
            }

            if (this.referenceResolver != null)
            {
                this.referenceResolver.Dispose();
                this.referenceResolver = null;
            }
        }
    }
}

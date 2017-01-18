// --------------------------------------------------------------------------------------------------------------------
// <copyright file="OnlineTrainerSettingsDownloader.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using Microsoft.Azure;
using Microsoft.WindowsAzure.ServiceRuntime;
using Microsoft.WindowsAzure.Storage;
using Microsoft.WindowsAzure.Storage.Blob;
using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Net;
using System.Reactive.Linq;
using System.Threading;
using System.Threading.Tasks;
using VW.Azure.Trainer;

namespace VW.Azure.Worker
{
    internal sealed class OnlineTrainerSettingsDownloader : IDisposable
    {
        internal static CloudBlob GetSettingsBlockBlob()
        {
            var storageAccount = CloudStorageAccount.Parse(CloudConfigurationManager.GetSetting("StorageConnectionString"));

            var blobClient = storageAccount.CreateCloudBlobClient();
            var container = blobClient.GetContainerReference(OnlineTrainerSettings.SettingsContainerName);
            return container.GetBlobReference(OnlineTrainerSettings.LatestClientSettingsBlobName);
        }

        public delegate void DownloadedEventHandler(object sender, byte[] data);

        public delegate void FailedEventHandler(object sender, Exception e);

        public event DownloadedEventHandler Downloaded;

        public event FailedEventHandler Failed;

        private IDisposable disposable;

        private CloudBlob settingsBlob;

        private string blobEtag;

        public OnlineTrainerSettingsDownloader(TimeSpan interval)
        {
            this.settingsBlob = GetSettingsBlockBlob();
            RoleEnvironment.Changed += RoleEnvironment_Changed;

            // run background thread
            var conn = Observable.Interval(interval)
                .SelectMany(_ => Observable.FromAsync(this.Execute))
                .Replay();

            this.disposable = conn.Connect();
        }

        private void RoleEnvironment_Changed(object sender, RoleEnvironmentChangedEventArgs e)
        {
            var change = e.Changes
                .OfType<RoleEnvironmentConfigurationSettingChange>()
                .FirstOrDefault(c => c.ConfigurationSettingName == "StorageConnectionString");

            if (change != null)
                this.settingsBlob = GetSettingsBlockBlob();
        }

        private async Task Execute(CancellationToken cancellationToken)
        {
            var uri = string.Empty;
            try
            {
                uri = this.settingsBlob.Uri.ToString();

                // avoid not found exception
                if (!await this.settingsBlob.ExistsAsync(cancellationToken))
                    return;

                if (this.settingsBlob.Properties != null)
                {
                    // if downloadImmediately is set to false, the downloader
                    // will not download the blob on first check, and on second check
                    // onwards, the blob must have changed before a download is triggered.
                    // this is to support caller who manually downloads the blob first for
                    // other purposes and do not want to redownload.

                    // avoid not modified exception
                    if (this.settingsBlob.Properties.ETag == this.blobEtag)
                        return;

                    var currentBlobEtag = this.blobEtag;
                    this.blobEtag = this.settingsBlob.Properties.ETag;
                }

                // download
                using (var ms = new MemoryStream())
                {
                    await this.settingsBlob.DownloadToStreamAsync(ms, cancellationToken);

                    Trace.TraceInformation("Retrieved new blob for {0}", this.settingsBlob.Uri);

                    this.Downloaded?.Invoke(this, ms.ToArray());
                }
            }
            catch (Exception ex)
            {
                if (ex is StorageException)
                {
                    RequestResult result = ((StorageException)ex).RequestInformation;
                    if (result.HttpStatusCode != (int)HttpStatusCode.NotFound)
                    {
                        Trace.TraceError(
                          "Failed to retrieve '{0}': {1}. {2}",
                          uri, ex.Message, result.HttpStatusMessage);
                    }

                }
                else
                    Trace.TraceError("Failed to retrieve '{0}': {1}", uri, ex.Message);

                this.Failed?.Invoke(this, ex);
            }
        }

        public void Dispose()
        {
            RoleEnvironment.Changed -= RoleEnvironment_Changed;

            if (this.disposable != null)
            {
                this.disposable.Dispose();
                this.disposable = null;
            }
        }
    }
}

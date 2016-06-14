// --------------------------------------------------------------------------------------------------------------------
// <copyright file="CheckpointOperation.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using Microsoft.ApplicationInsights.DataContracts;
using Microsoft.WindowsAzure.Storage.Blob;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Threading.Tasks;

namespace VowpalWabbit.Azure.Trainer.Operations
{
    internal sealed class CheckpointEvent
    {
        public override string ToString()
        {
            return "Checkpoint request";
        }
    }

    internal sealed class CheckpointData
    {
        internal byte[] Model { get; set; }

        internal byte[] EvalModel { get; set; }

        internal string TrackbackList { get; set; }

        internal bool UpdateClientModel { get; set; }

        internal string State { get; set; }

        internal string Timestamp { get; set; }
    }

    internal partial class Learner
    {
        internal bool ShouldCheckpoint()
        {
            // don't checkpoint if we didn't see any valid events.
            return this.trackbackList.Count > 0 && 
                this.settings.CheckpointPolicy.ShouldCheckpointAfterExample(1);
        }

        internal CheckpointData CreateCheckpointData()
        {
            // TODO: checkpoint resolver state.
            var data = new CheckpointData
            {
                TrackbackList = string.Join("\n", this.trackbackList),
                State = JsonConvert.SerializeObject(this.State),
                Timestamp = DateTime.UtcNow.ToString("yyyyMMdd/HHmmss", CultureInfo.InvariantCulture)
            };
            
            this.trackbackList.Clear();

            var modelId = Guid.NewGuid().ToString();
            using (var memStream = new MemoryStream())
            {
                this.vw.ID = modelId;
                this.vw.SaveModel(memStream);
                data.Model = memStream.ToArray();

                return data;
            }
        }

        internal async Task Checkpoint(object obj)
        {
            try
            {
                var data = obj as CheckpointData;
                if (data == null)
                {
                    this.telemetry.TrackTrace($"Received invalid data: {data}");
                    return;
                }

                var modelName = data.Timestamp + "/model";

                this.telemetry.TrackTrace(
                    "CheckPoint " + modelName,
                    SeverityLevel.Information);

                var container = this.blobClient.GetContainerReference(this.settings.StorageContainerName);
                await container.CreateIfNotExistsAsync();

                // save model to storage account

                // save trackback file to storage account
                var trackbackName = string.Format(CultureInfo.InvariantCulture, "{0}/model.trackback", data.Timestamp);
                var trackbackBlob = container.GetBlockBlobReference(trackbackName);

                // keep a history of state files
                var stateName = string.Format(CultureInfo.InvariantCulture, "{0}/{1}", data.Timestamp, Learner.StateBlobName);
                var stateBlob = container.GetBlockBlobReference(stateName);

                await Task.WhenAll(
                    trackbackBlob.UploadTextAsync(data.TrackbackList),
                    stateBlob.UploadTextAsync(data.State));

                var modelBlob = await ExportModel(container, data.Model, modelName);

                // update the fast recovery state file
                var latestState = container.GetBlockBlobReference(Learner.StateBlobName);
                await latestState.StartCopyAsync(stateBlob);

                if (data.UpdateClientModel)
                {
                    // update latest model
                    var latestModel = await this.GetLatestModelBlob();
                    await latestModel.StartCopyAsync(modelBlob);
                }
            }
            catch (Exception ex)
            {
                this.telemetry.TrackException(ex);
            }
        }

        private async Task<CloudBlockBlob> ExportModel(CloudBlobContainer container, byte[] model, string modelName)
        {
            var modelBlob = container.GetBlockBlobReference(modelName);
            await modelBlob.UploadFromByteArrayAsync(model, 0, model.Length);

            this.telemetry.TrackTrace(
                string.Format("Model Save {0}", modelBlob.Uri),
                SeverityLevel.Information,
                new Dictionary<string, string>
                {
                    { "Size", model.Length.ToString() },
                    { "Uri", modelBlob.Uri.ToString() }
                });

            return modelBlob;
        }
    }
}

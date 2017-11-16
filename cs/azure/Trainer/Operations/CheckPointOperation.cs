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
using VW.Azure.Trainer.Data;

namespace VW.Azure.Trainer
{
    internal partial class Learner
    {
        internal bool ShouldCheckpoint(int numExamples)
        {
            // don't checkpoint if we didn't see any valid events.
            return this.trackbackList.Count > 0 && 
                this.settings.CheckpointPolicy.ShouldCheckpointAfterExample(numExamples);
        }

        internal CheckpointData CreateCheckpointData(bool updateClientModel)
        {
            // TODO: checkpoint resolver state.

            var data = new CheckpointData
            {
                Timestamp = DateTime.UtcNow.ToString("yyyyMMdd/HHmmss", CultureInfo.InvariantCulture),
                UpdateClientModel = updateClientModel,
                StartDateTime = this.startDateTime
            };

            var modelId = Guid.NewGuid().ToString();

            // store the model name
            this.state.ModelName = $"{data.Timestamp}/model";
            data.State = JsonConvert.SerializeObject(this.State);
            data.TrackbackCount = this.trackbackList.Count;
            data.TrackbackList = $"modelid: {modelId}\n" + string.Join("\n", this.trackbackList);

            this.trackbackList.Clear();

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

                var modelBlob = await ExportModel(container, data.Model, modelName, data.TrackbackCount);

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

        private async Task<CloudBlockBlob> ExportModel(CloudBlobContainer container, byte[] model, string modelName, int numExamples)
        {
            var modelBlob = container.GetBlockBlobReference(modelName);
            await modelBlob.UploadFromByteArrayAsync(model, 0, model.Length);

            this.telemetry.TrackTrace(
                $"Model Save {modelBlob.Uri}",
                SeverityLevel.Information,
                new Dictionary<string, string>
                {

                    { "Size", model.Length.ToString() },
                    { "Uri", modelBlob.Uri.ToString() },
                    { "Examples added", numExamples.ToString() }
                });

            return modelBlob;
        }
    }
}

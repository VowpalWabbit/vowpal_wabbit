// --------------------------------------------------------------------------------------------------------------------
// <copyright file="Learner.cs">
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
using System.Linq;
using System.Runtime.Caching;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using VW;
using VW.Serializer;

namespace VW.Azure.Trainer
{
    internal partial class Learner
    {
        public async Task FindAndResumeFromState()
        {
            var container = this.blobClient.GetContainerReference(this.settings.StorageContainerName);
            await container.CreateIfNotExistsAsync();

            var stateBlob = container.GetBlockBlobReference(Learner.StateBlobName);
            if (await stateBlob.ExistsAsync() && 
                await this.TryResumeFromState(stateBlob))
                return;

            // find days
            var dayDirectories = from c in container.ListBlobs(useFlatBlobListing: false)
                                    let dir = c as CloudBlobDirectory
                                    where dir != null && dir.Prefix.Length >= 8
                                    let date = DateTime.ParseExact(dir.Prefix.Substring(0, 8), "yyyyMMdd", CultureInfo.InvariantCulture)
                                    orderby date descending
                                    select dir;

            foreach (var day in dayDirectories)
            {
                // find state files
                var states = from f in container.ListBlobs(day.Prefix, useFlatBlobListing: true)
                                let file = f as CloudBlockBlob
                                where file != null
                                let match = Regex.Match(file.Name, @"^\d{8}/(\d{6})/" + Learner.StateBlobName + "$")
                                where match.Success
                                let time = DateTime.ParseExact(match.Groups[1].Value, "HHmmss", CultureInfo.InvariantCulture)
                                orderby time descending
                                select file;

                foreach (var file in states)
                {
                    if (await TryResumeFromState(file))
                        return;
                }
            }

            // unable to find a model, fallback to fresh start
            this.FreshStart();
        }

        internal void FreshStart(OnlineTrainerState state = null, byte[] model = null)
        {
            if (state == null)
                state = new OnlineTrainerState();

            this.telemetry.TrackTrace("Fresh Start", SeverityLevel.Information);

            // start from scratch
            this.state = state;

            // save extra state so learning can be resumed later with new data
            var baseArguments = "--save_resume";

            var settings = model == null ?
                new VowpalWabbitSettings(baseArguments + " " + this.settings.Metadata.TrainArguments) :
                new VowpalWabbitSettings(baseArguments) { ModelStream = new MemoryStream(model) };

            this.InitializeVowpalWabbit(settings);
        }

        private async Task<bool> TryLoadModel()
        {
            // find the model blob
            if (string.IsNullOrEmpty(this.state.ModelName))
            {
                this.telemetry.TrackTrace("Model not specified");
                return false;
            }

            var container = this.blobClient.GetContainerReference(this.settings.StorageContainerName);
            if (!await container.ExistsAsync())
            {
                this.telemetry.TrackTrace($"Storage container missing '{this.settings.StorageContainerName}'");
                return false;
            }

            var modelBlob = container.GetBlockBlobReference(this.state.ModelName);
            if (!await modelBlob.ExistsAsync())
            {
                this.telemetry.TrackTrace($"Model blob '{this.state.ModelName}' is missing");
                return false;
            }

            // load the model
            using (var modelStream = await modelBlob.OpenReadAsync())
            {
                this.InitializeVowpalWabbit(new VowpalWabbitSettings { ModelStream = modelStream });
                this.telemetry.TrackTrace($"Model loaded {this.state.ModelName}", SeverityLevel.Verbose);

                // validate that the loaded VW model has the same settings as requested by C&C
                var newSettings = new VowpalWabbitSettings(this.settings.Metadata.TrainArguments);
                using (var newVW = new VW.VowpalWabbit(newSettings))
                {
                    newVW.ID = this.vw.ID;

                    // save the VW instance to a model and load again to get fully expanded parameters.
                    string newVWarguments;
                    using (var tempModel = new MemoryStream())
                    {
                        newVW.SaveModel(tempModel);
                        tempModel.Position = 0;

                        using (var tempVW = new VW.VowpalWabbit(new VowpalWabbitSettings { ModelStream = tempModel }))
                        {
                            newVWarguments = CleanVowpalWabbitArguments(tempVW.Arguments.CommandLine);
                        }
                    }

                    var oldVWarguments = CleanVowpalWabbitArguments(this.vw.Arguments.CommandLine);

                    // this is the expanded command line
                    if (newVWarguments != oldVWarguments)
                    {
                        this.telemetry.TrackTrace("New VowpalWabbit settings found. Discarding existing model",
                            SeverityLevel.Information,
                            new Dictionary<string, string>
                            {
                            { "TrainArguments", newVW.Arguments.CommandLine },
                            { "NewExpandedArguments", newVWarguments },
                            { "OldExpandedArgumentsCleaned", oldVWarguments },
                            { "OldExpandedArguments", this.vw.Arguments.CommandLine },
                            });

                        // discard old, use fresh
                        this.vw.Dispose();
                        this.vw = null;

                        this.InitializeVowpalWabbit(newSettings);
                    }
                }
            }

            // store the initial model
            this.settings.InitialVowpalWabbitModel = this.state.ModelName;

            return true;
        }

        // --max_prediction 111.670006 
        private static readonly Regex RegexMaxPrediction = new Regex("--max_prediction\\s+\\S+(\\s?|$)", RegexOptions.Compiled);

        private static string CleanVowpalWabbitArguments(string args)
        {
            return RegexMaxPrediction.Replace(args, " ");
        }
        
        private void InitializeVowpalWabbit(VowpalWabbitSettings vwSettings)
        {
            if (this.settings.EnableExampleTracing)
            {
                vwSettings.EnableStringExampleGeneration = true;
                vwSettings.EnableStringFloatCompact = true;
            }

            vwSettings.EnableThreadSafeExamplePooling = true;
            vwSettings.MaxExamples = 64 * 1024;

            try
            {
                this.startDateTime = DateTime.UtcNow;
                this.vw = new VW.VowpalWabbit(vwSettings);
                var cmdLine = vw.Arguments.CommandLine;

                if (!(cmdLine.Contains("--cb_explore") || cmdLine.Contains("--cb_explore_adf")))
                    throw new ArgumentException("Only cb_explore and cb_explore_adf are supported");
            }
            catch (Exception ex)
            {
                this.telemetry.TrackException(ex, new Dictionary<string, string>
                {
                    { "help", "Invalid model. For help go to https://github.com/JohnLangford/vowpal_wabbit/wiki/Azure-Trainer" }
                });

                throw ex;
            }

            this.referenceResolver = new VowpalWabbitJsonReferenceResolver(
                this.delayedExampleCallback,
                cacheRequestItemPolicyFactory:
                    key => new CacheItemPolicy()
                    {
                        SlidingExpiration = TimeSpan.FromHours(1),
                        RemovedCallback = this.CacheEntryRemovedCallback
                    });

            //this.vwAllReduce = new VowpalWabbitThreadedLearning(vwSettings.ShallowCopy(
            //    maxExampleQueueLengthPerInstance: 4*1024,
            //    parallelOptions: new ParallelOptions
            //    {
            //        MaxDegreeOfParallelism = 2,
            //    },
            //    exampleDistribution: VowpalWabbitExampleDistribution.RoundRobin,
            //    exampleCountPerRun: 128 * 1024));
        }

        private void CacheEntryRemovedCallback(CacheEntryRemovedArguments arguments)
        {
            switch (arguments.RemovedReason)
            {
                case CacheEntryRemovedReason.Evicted:
                case CacheEntryRemovedReason.Expired:
                    // free memory
                    var serializer = (VowpalWabbitJsonSerializer)arguments.CacheItem.Value;
                    serializer.Dispose();
                        
                    this.perfCounters.Feature_Requests_Pending.IncrementBy(-1);
                    this.perfCounters.Feature_Requests_Discarded.Increment();
                    break;
            }
        }

        private async Task<bool> TryResumeFromState(CloudBlockBlob stateBlob)
        {
            using (var stream = await stateBlob.OpenReadAsync())
            using (var reader = new JsonTextReader(new StreamReader(stream)))
            {
                var jsonSerializer = JsonSerializer.CreateDefault();
                this.state = jsonSerializer.Deserialize<OnlineTrainerState>(reader);
            }

            this.telemetry.TrackTrace(
                $"Resume from '{stateBlob.Uri}'", 
                SeverityLevel.Verbose,
                this.state.PartitionsDetailed);

            return await this.TryLoadModel();
        }
    }
}

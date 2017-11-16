using Microsoft.ApplicationInsights.DataContracts;
using Microsoft.ApplicationInsights.Extensibility;
using Microsoft.ServiceBus.Messaging;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.WindowsAzure.Storage;
using Microsoft.WindowsAzure.Storage.Blob;
using MoreLinq;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using VW;
using VW.Azure.Trainer;
using VW.Azure.Trainer.Checkpoint;
using VW.Serializer;

namespace cs_unittest
{
    [TestClass]
    public class TestAzure
    {
        public class SharedFeatures
        {
            public string Location { get; set; }
        }

        public class ActionNamespace
        {
            public float Category { get; set; }
        }

        public class ActionFeatures
        {
            [JsonProperty("b")]
            public ActionNamespace Namespace { get; set; }

            // TODO: _tag
        }

        public class Context
        {
            // TODO: _ProbabilityOfDrop

            [JsonIgnore]
            public int Index { get; set; }

            [JsonIgnore]
            public string JSON
            {
                get { return JsonConvert.SerializeObject(this); }
            }

            [JsonIgnore]
            public byte[] JSONAsBytes
            {
                get { return Encoding.UTF8.GetBytes(this.JSON); }
            }

            [JsonProperty("_eventid")]
            public string EventId { get; set; }

            [JsonProperty("_timestamp")]
            public DateTime Timestamp { get; set; }

            [JsonProperty("_a")]
            public int[] ActionIndicies { get; set; }

            [JsonProperty("_p")]
            public float[] Probabilities { get; set; }

            [JsonProperty("_label_action")]
            public int LabelAction { get; set; }

            [JsonProperty("_label_cost")]
            public float LabelCost { get; set; }

            [JsonProperty("_label_probability")]
            public float LabelProbability { get; set; }

            [JsonProperty("_labelindex")]
            public int LabelIndex { get; set; }

            [JsonProperty("a")]
            public SharedFeatures Shared { get; set; }

            [JsonProperty("_multi")]
            public ActionFeatures[] Actions { get; set; }
        }

        private static string GetConfiguration(string name)
        {
            var value = Environment.GetEnvironmentVariable(name);
            if (!string.IsNullOrEmpty(value))
                return value.Trim();

            var path = Directory.GetCurrentDirectory();
            do
            {
                var filename = Path.Combine(path, "vw_azure.config");
                if (File.Exists(filename))
                {
                    var q = from line in File.ReadAllLines(filename)
                            let m = Regex.Match(line, @"^(\S+)\s*=(.*)$")
                            where m.Success
                            where m.Groups[1].Value == name
                            select m.Groups[2].Value;

                    value = q.FirstOrDefault();
                    if (!string.IsNullOrEmpty(value))
                        return value.Trim();
                }

                var di = Directory.GetParent(path);
                if (di == null)
                    Assert.Fail($"Configuration variable '{name}' not found. Search for environment variable or vw_azure.config");

                path = di.FullName;
            }
            while (true);
        }

        private class OnlineTrainerWrapper : IDisposable
        {
            internal string trainArguments;
            internal OnlineTrainerBlobs Blobs;

            private string storageConnectionString = GetConfiguration("storageConnectionString");
            private string inputEventHubConnectionString = GetConfiguration("inputEventHubConnectionString");
            private string evalEventHubConnectionString = GetConfiguration("evalEventHubConnectionString");
            private SynchronizedCollection<ExceptionTelemetry> exceptions;
            private LearnEventProcessorHost trainProcesserHost;

            internal OnlineTrainerWrapper(string trainArguments)
            {
                this.trainArguments = trainArguments;

                Blobs = new OnlineTrainerBlobs(storageConnectionString);

                // register with AppInsights to collect exceptions
                // need to set the instrumentation key, otherwise the processor is ignored.
                TelemetryConfiguration.Active.InstrumentationKey = "00000000-0000-0000-0000-000000000000";
                exceptions = new SynchronizedCollection<ExceptionTelemetry>();
                var builder = TelemetryConfiguration.Active.TelemetryProcessorChainBuilder;
                builder.Use((next) => new TestTelemetryProcessor(next, exceptions));
                builder.Build();
            }

            void AssertNoExceptionsThroughAppInsights()
            {
                Assert.AreEqual(0, exceptions.Count, string.Join("\n", exceptions.Select(e => e.Exception.Message + " " + e.Message)));
            }

            internal async Task StartAsync(ICheckpointPolicy checkpointPolicy)
            {
                trainProcesserHost = new LearnEventProcessorHost();
                await trainProcesserHost.StartAsync(new OnlineTrainerSettingsInternal
                {
                    CheckpointPolicy = checkpointPolicy,
                    JoinedEventHubConnectionString = inputEventHubConnectionString,
                    EvalEventHubConnectionString = evalEventHubConnectionString,
                    StorageConnectionString = storageConnectionString,
                    Metadata = new OnlineTrainerSettings
                    {
                        ApplicationID = "vwunittest",
                        TrainArguments = trainArguments
                    },
                    EnableExampleTracing = false,
                    EventHubStartDateTimeUtc = DateTime.UtcNow // ignore any events that arrived before this time
                });

                AssertNoExceptionsThroughAppInsights();
            }

            internal async Task PollTrainerCheckpoint(Predicate<OnlineTrainerBlobs> predicate)
            {
                // wait for trainer to checkpoint
                await Blobs.PollTrainerCheckpoint(exceptions, predicate);
            }

            internal void SendData(IEnumerable<Context> data)
            {
                // send events to event hub
                var eventHubInputClient = EventHubClient.CreateFromConnectionString(inputEventHubConnectionString);
                data.ForEach(c => eventHubInputClient.Send(new EventData(c.JSONAsBytes) { PartitionKey = c.Index.ToString() }));
            }

            public void Dispose()
            {
                if (trainProcesserHost != null)
                {
                    trainProcesserHost.Dispose();
                    trainProcesserHost = null;
                }
            }

            internal void TrainOffline(string message, string modelId, Dictionary<string, Context> data, IEnumerable<string> eventOrder, Uri onlineModelUri, string trainArguments = null)
            {
                // allow override
                if (trainArguments == null)
                    trainArguments = this.trainArguments;

                // train model offline using trackback
                var settings = new VowpalWabbitSettings(trainArguments + $" --id {modelId} --save_resume --preserve_performance_counters -f offline.model");
                using (var vw = new VowpalWabbitJson(settings))
                {
                    foreach (var id in eventOrder)
                    {
                        var json = data[id].JSON;

                        var progressivePrediction = vw.Learn(json, VowpalWabbitPredictionType.ActionProbabilities);
                        // TODO: validate eval output
                    }
                }

                using (var vw = new VowpalWabbit("-i offline.model --save_resume --readable_model offline.model.txt -f offline.reset_perf_counters.model"))
                { }

                Blobs.DownloadFile(onlineModelUri, "online.model");
                using (var vw = new VowpalWabbit("-i online.model --save_resume --readable_model online.model.txt -f online.reset_perf_counters.model"))
                { }


                // validate that the model is the same
                CollectionAssert.AreEqual(
                    File.ReadAllBytes("offline.reset_perf_counters.model"),
                    File.ReadAllBytes("online.reset_perf_counters.model"),
                    $"{message}. Offline and online model differs. Compare online.model.txt with offline.model.txt to compare");
            }
        }

        private async Task<OnlineTrainerWrapper> RunTrainer(string args, IEnumerable<Context> data, Dictionary<string, Context> dataMap, int expectedNumStates, bool cleanBlobs)
        {
            var trainer = new OnlineTrainerWrapper("--cb_explore_adf --epsilon 0.1 -q ab -l 0.1");

            if (cleanBlobs)
                trainer.Blobs.Cleanup().Wait();

            // start listening for event hub
            await trainer.StartAsync(new CountingCheckpointPolicy(100));

            // send data to event hub
            trainer.SendData(data);

            await trainer.PollTrainerCheckpoint(blobs => 
                blobs.ModelBlobs.Count == expectedNumStates && 
                blobs.ModelTrackbackBlobs.Count == expectedNumStates &&
                blobs.StateJsonBlobs.Count == expectedNumStates);

            // download & parse trackback file
            trainer.Blobs.DownloadTrackbacksOrderedByTime();

            foreach (var trackback in trainer.Blobs.Trackbacks)
                // due to checkpoint policy = 100
                Assert.AreEqual(100, trackback.EventIds.Count, $"{trackback.Blob.Uri} does not contain the expected 100 events. Actual: {trackback.EventIds.Count}");

            return trainer;
        }

        [TestMethod]
        [TestCategory("NotOnVSO")]
        [TestCategory("Vowpal Wabbit")]
        [Ignore]
        public async Task TestAzureTrainerRestart()
        {
            // generate data
            var data = GenerateData(600).ToList();
            var dataMap = data.ToDictionary(d => d.EventId, d => d);

            var args = "--cb_explore_adf --epsilon 0.1 -q ab -l 0.1";

            using (var trainer = await RunTrainer(args, data.Take(220), dataMap, expectedNumStates: 2, cleanBlobs: true))
            {
                trainer.TrainOffline("produce the 1st model", trainer.Blobs.Trackbacks[0].ModelId, dataMap, trainer.Blobs.Trackbacks[0].EventIds, trainer.Blobs.ModelBlobs[0].Uri);
                // keep model for subsequent training
                File.Copy("offline.model", "split1.model", overwrite: true);

                trainer.TrainOffline("produce the 2nd model by training through all events", trainer.Blobs.Trackbacks[1].ModelId, dataMap, trainer.Blobs.Trackbacks.SelectMany(t => t.EventIds), trainer.Blobs.ModelBlobs[1].Uri);
                File.Copy("offline.model", "split2.model", overwrite: true);

                trainer.TrainOffline("produce the 2nd model by starting from the 1st and then continuing", trainer.Blobs.Trackbacks[1].ModelId, dataMap, trainer.Blobs.Trackbacks[1].EventIds, trainer.Blobs.ModelBlobs[1].Uri, "-i split1.model -l 0.1");
            }

            // restart trainer and resume from split2.model, covering "fresh -> load" transition
            using (var trainer = await RunTrainer(args, data.Skip(220).Take(120), dataMap, expectedNumStates: 3, cleanBlobs: false))
            {
                var lastTrackback = trainer.Blobs.Trackbacks.Last();
                var lastBlob = trainer.Blobs.ModelBlobs.Last();
                trainer.TrainOffline("produce the 3rd model by training through all events", lastTrackback.ModelId, dataMap, trainer.Blobs.Trackbacks.SelectMany(t => t.EventIds), lastBlob.Uri);
                trainer.TrainOffline("produce the 3rd model by starting from the 2nd and then continuing", lastTrackback.ModelId, dataMap, lastTrackback.EventIds, lastBlob.Uri, "-i split2.model -l 0.1");

                File.Copy("offline.model", "split3.model", overwrite: true);
            }

            // restart ones more to cover "load -> save"
            using (var trainer = await RunTrainer(args, data.Skip(340).Take(120), dataMap, expectedNumStates: 4, cleanBlobs: false))
            {
                var lastTrackback = trainer.Blobs.Trackbacks.Last();
                var lastBlob = trainer.Blobs.ModelBlobs.Last();
                trainer.TrainOffline("produce the 4th model by training through all events", lastTrackback.ModelId, dataMap, trainer.Blobs.Trackbacks.SelectMany(t => t.EventIds), lastBlob.Uri);
                trainer.TrainOffline("produce the 4th model by starting from the 3rd and then continuing", lastTrackback.ModelId, dataMap, lastTrackback.EventIds, lastBlob.Uri, "-i split3.model -l 0.1");
            }
        }

        [TestMethod]
        [TestCategory("NotOnVSO")]
        [TestCategory("Vowpal Wabbit")]
        [Ignore]
        public async Task TestAzureTrainer()
        {
            using (var trainer = new OnlineTrainerWrapper("--cb_explore_adf --epsilon 0.2 -q ab"))
            {
                trainer.Blobs.Cleanup().Wait();

                // generate data
                var data = GenerateData(100).ToList();
                var dataMap = data.ToDictionary(d => d.EventId, d => d);

                // start listening for event hub
                await trainer.StartAsync(new CountingCheckpointPolicy(data.Count));

                // send data to event hub
                trainer.SendData(data);

                // wait for trainer to checkpoint
                await trainer.PollTrainerCheckpoint(blobs => blobs.ModelBlobs.Count > 0 && blobs.ModelTrackbackBlobs.Count > 0 && blobs.StateJsonBlobs.Count > 0 );

                // download & parse trackback file
                trainer.Blobs.DownloadTrackbacksOrderedByTime();
                Assert.AreEqual(1, trainer.Blobs.Trackbacks.Count);

                var trackback = trainer.Blobs.Trackbacks[0];
                Assert.AreEqual(data.Count, trackback.EventIds.Count);
                Assert.AreEqual(1, trainer.Blobs.ModelBlobs.Count);

                trainer.TrainOffline("train a model for this set of events", trackback.ModelId, dataMap, trackback.EventIds, trainer.Blobs.ModelBlobs[0].Uri);
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void TestCbProgressiveValidation()
        {
            int numExamples = 1024;
            foreach (var cbType in new[] { "ips", "dr", "mtr" })
            {
                var trainArguments = $"--cb_explore_adf --epsilon 0.1 --bag 3 -q ab --power_t 0 -l 0.1 --cb_type {cbType} --random_seed 50";
                int[] topActionCounts = new int[3];
                using (var vw1 = new VowpalWabbitJson(trainArguments))
                using (var vw2 = new VowpalWabbitJson(trainArguments))
                {
                    foreach (var ex in GenerateData(numExamples))
                    {
                        var json = ex.JSON;

                        var pred1_a = vw1.Predict(json, VowpalWabbitPredictionType.ActionProbabilities);
                        var pred1_b = vw1.Learn(json, VowpalWabbitPredictionType.ActionProbabilities);

                        var pred2 = vw2.Learn(json, VowpalWabbitPredictionType.ActionProbabilities);

                        AreEqual(pred1_a, pred2, cbType);
                        AreEqual(pred1_b, pred2, cbType);

                        topActionCounts[pred2[0].Action]++;
                        
                        //Debug.WriteLine(json);
                        //Debug.WriteLine("Prob1.pred:  " + string.Join(",", pred1_a.Select(a=>$"{a.Action}:{a.Score}")));
                        //Debug.WriteLine("Prob1.learn: " + string.Join(",", pred1_b.Select(a=>$"{a.Action}:{a.Score}")));
                        //Debug.WriteLine("Prob2.learn: " + string.Join(",", pred2.Select(a=>$"{a.Action}:{a.Score}")));
                        //Debug.WriteLine("");
                    }
                }

                foreach (var count in topActionCounts)
                    Assert.IsTrue(count < numExamples * 0.8, $"Unexpected action distribution: {count}");
                Debug.WriteLine($"cb_types: {cbType} " + string.Join(",", topActionCounts.Select((count, i) => $"{i}:{count}")));
            }
        }

        private static void AreEqual(ActionScore[] expected, ActionScore[] actual, string cbType)
        {
            Assert.AreEqual(expected.Length, actual.Length);
            for (int i = 0; i < expected.Length; i++)
            {
                Assert.AreEqual(expected[i].Action, actual[i].Action, $"cb_type: {cbType} Action mismatch at index {i}. Expected: {expected[i].Action} Actual: {actual[i].Action}");
                Assert.AreEqual(expected[i].Score, actual[i].Score, $"cb_type: {cbType} Score mismatch at index {i}. Expected: {expected[i].Score} Actual: {actual[i].Score}");
            }
        }

        internal class Trackback
        {
            public IListBlobItem Blob;

            public string ModelId;

            public List<string> EventIds;

            public DateTime Timestamp;
        }

        private static IEnumerable<Context> GenerateData(int n)
        {
            var random = new Random(42);
            var locations = new[] { "east", "west" };

            for (int i = 0; i < n; i++)
            {
                var action = random.Next(2); // (i % 2);
                var prob = (float)random.NextDouble();
                var probs = action == 0 ? new[] { prob, 1 - prob } : new[] { 1 - prob, prob };

                yield return new Context
                {
                    Index = i,
                    ActionIndicies = new[] { 1, 2 },
                    Probabilities = new[] { 0.5f, 0.5f },// probs,
                    Timestamp = DateTime.UtcNow,
                    EventId = Guid.NewGuid().ToString("n"),
                    LabelAction = action + 1, // random.Next(1, 3), //action + 1,
                    LabelCost = random.Next(4) - 2,
                    LabelIndex = action, // action,
                    LabelProbability = prob,
                    Shared = new SharedFeatures { Location = locations[random.Next(2)] },
                    Actions = new[]
                    {
                        new ActionFeatures { Namespace = new ActionNamespace { Category = (float)random.NextDouble() } },
                        new ActionFeatures { Namespace = new ActionNamespace { Category = 1.5f + (float)random.NextDouble() } },
                        new ActionFeatures { Namespace = new ActionNamespace { Category = 1.5f + (float)random.NextDouble() } }
                        //new ActionFeatures { Namespace = new ActionNamespace { Category = "games" } },
                        //new ActionFeatures { Namespace = new ActionNamespace { Category = "news" } }
                    }
                };
            }
        }

        internal class OnlineTrainerBlobs
        {
            internal CloudBlobClient BlobClient;
            internal CloudBlobContainer ModelContainer;
            internal CloudBlockBlob CurrentModel;
            internal CloudBlobContainer TrainerContainer;
            internal List<IListBlobItem> ModelBlobs;
            internal List<IListBlobItem> ModelTrackbackBlobs;
            internal List<IListBlobItem> StateJsonBlobs;
            internal List<Trackback> Trackbacks;

            internal OnlineTrainerBlobs(string storageConnectionString)
            {
                this.BlobClient = CloudStorageAccount.Parse(storageConnectionString).CreateCloudBlobClient();

                this.ModelContainer = this.BlobClient.GetContainerReference("mwt-models");
                this.CurrentModel = this.ModelContainer.GetBlockBlobReference("current");
                this.TrainerContainer = this.BlobClient.GetContainerReference("onlinetrainer");
            }

            internal void DownloadFile(Uri uri, string filename)
            {
                new CloudBlob(uri, BlobClient.Credentials).DownloadToFile(filename, FileMode.Create);
            }

            internal async Task Cleanup()
            {
                // don't delete the container as this will trigger a conflict unless we wait...
                if (this.ModelContainer.Exists())
                    this.CurrentModel.DeleteIfExists();

                if (this.TrainerContainer.Exists())
                {
                    // don't delete the container as this will trigger a conflict unless we wait...
                    await Task.WhenAll(
                        this.TrainerContainer.ListBlobs(useFlatBlobListing: true)
                            .Select(blob => new CloudBlob(blob.Uri, this.BlobClient.Credentials).DeleteIfExistsAsync()));
                }
            }

            internal async Task PollTrainerCheckpoint(SynchronizedCollection<ExceptionTelemetry> exceptions, Predicate<OnlineTrainerBlobs> predicate)
            {
                // wait for files to show up
                for (int i = 0; i < 30; i++)
                {
                    await Task.Delay(TimeSpan.FromSeconds(1));

                    if (exceptions.Count > 0)
                        Assert.Fail(string.Join(";", exceptions.Select(e => e.Message)));

                    // mwt-models
                    if (!this.ModelContainer.Exists())
                        continue;

                    // mwt-models/current
                    if (!this.CurrentModel.Exists())
                        continue;

                    // onlinetrainer
                    if (!this.TrainerContainer.Exists())
                        continue;

                    // onlinetrainer/20161128/002828
                    var blobs = this.TrainerContainer.ListBlobs(useFlatBlobListing: true);
                    this.ModelBlobs = blobs.Where(b => b.Uri.ToString().EndsWith("model"))
                        .OrderBy(uri => DateTime.ParseExact(uri.Parent.Prefix, "yyyyMMdd/HHmmss/", CultureInfo.InvariantCulture))
                        .ToList();
                    this.ModelTrackbackBlobs = blobs.Where(b => b.Uri.ToString().EndsWith("model.trackback")).ToList();
                    this.StateJsonBlobs = blobs.Where(b => !string.IsNullOrEmpty(b.Parent.Prefix) && b.Uri.ToString().EndsWith("state.json")).ToList();

                    if (predicate(this))
                        return;
                }

                Assert.Fail("Trainer didn't produce checkpoints");
            }

            internal void DownloadTrackbacksOrderedByTime()
            {
                this.Trackbacks = this.ModelTrackbackBlobs.Select(b =>
                {
                    var trackbackStr = this.DownloadNonEmptyBlob(b);

                    var trackback = trackbackStr.Split('\n');
                    // modelid: faf5e313-46bb-4852-af05-576c3a1c2c67
                    var m = Regex.Match(trackback[0], "^modelid: (.+)$");

                    Assert.IsTrue(m.Success, $"Unable to extract model id from trackback file. Line '{trackback[0]}'");

                    return new Trackback
                    {
                        Blob = b,
                        Timestamp = DateTime.ParseExact(b.Parent.Prefix, "yyyyMMdd/HHmmss/", CultureInfo.InvariantCulture),
                        ModelId = m.Groups[1].Value,
                        EventIds = trackback.Skip(1).ToList()
                    };
                })
                .OrderBy(x => x.Timestamp).ToList();
            }

            internal string DownloadNonEmptyBlob(IListBlobItem blob)
            {
                using (var memStream = new MemoryStream())
                {
                    new CloudBlob(blob.Uri, this.BlobClient.Credentials).DownloadToStream(memStream);
                    var content = Encoding.UTF8.GetString(memStream.ToArray());
                    Assert.IsTrue(!string.IsNullOrWhiteSpace(content), $"File is empty: '{blob.Uri}'");

                    return content;
                }
            }
        }

        //[TestMethod]
        //public void TestQueueDictionary()
        //{
        //    var qd = new QueueDictionary<int, string>();

        //    Assert.IsNull(qd.Remove(1));
        //    qd.Enqueue(1, "foo");

        //    Assert.AreEqual("foo", qd.Remove(1));
        //    Assert.AreEqual(0, qd.DequeueIf(_ => true).Count());

        //    qd.Enqueue(1, "foo");
        //    qd.Enqueue(2, "bar");

        //    Assert.AreEqual(1, qd.DequeueIf(key => key == "foo").Count());
        //    Assert.AreEqual("foo", qd.Remove(1));
        //}
    }
}

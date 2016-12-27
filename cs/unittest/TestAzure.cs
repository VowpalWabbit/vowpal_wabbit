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

        [TestMethod]
        // TODO: figure issue in VSO
        [TestCategory("NotOnBuild")]
        [Ignore]
        public async Task TestAzureTrainer()
        {
            var storageConnectionString = GetConfiguration("storageConnectionString");
            var inputEventHubConnectionString = GetConfiguration("inputEventHubConnectionString");
            var evalEventHubConnectionString = GetConfiguration("evalEventHubConnectionString");

            var trainArguments = "--cb_explore_adf --epsilon 0.2 -q ab";

            // register with AppInsights to collect exceptions
            var exceptions = RegisterAppInsightExceptionHook();

            // cleanup blobs
            var blobs = new ModelBlobs(storageConnectionString);
            await blobs.Cleanup();

            var data = GenerateData(100).ToDictionary(d => d.EventId, d => d);

            // start listening for event hub
            using (var trainProcesserHost = new LearnEventProcessorHost())
            {
                await trainProcesserHost.StartAsync(new OnlineTrainerSettingsInternal
                {
                    CheckpointPolicy = new CountingCheckpointPolicy(data.Count),
                    JoinedEventHubConnectionString = inputEventHubConnectionString,
                    EvalEventHubConnectionString = evalEventHubConnectionString,
                    StorageConnectionString = storageConnectionString,
                    Metadata = new OnlineTrainerSettings
                    {
                        ApplicationID = "vwunittest",
                        TrainArguments = trainArguments
                    },
                    EnableExampleTracing = true,
                    EventHubStartDateTimeUtc = DateTime.UtcNow // ignore any events that arrived before this time
                });

                // send events to event hub
                var eventHubInputClient = EventHubClient.CreateFromConnectionString(inputEventHubConnectionString);
                data.Values.ForEach(c => eventHubInputClient.Send(new EventData(c.JSONAsBytes) { PartitionKey = c.Index.ToString() }));

                // wait for trainer to checkpoint
                await blobs.PollTrainerCheckpoint(exceptions);

                // download & parse trackback file
                var trackback = blobs.DownloadTrackback();
                Assert.AreEqual(data.Count, trackback.EventIds.Count);

                // train model offline using trackback
                var settings = new VowpalWabbitSettings(trainArguments + $" --id {trackback.ModelId} --save_resume --readable_model offline.json.model.txt -f offline.json.model");
                using (var vw = new VowpalWabbitJson(settings))
                {
                    foreach (var id in trackback.EventIds)
                    {
                        var json = data[id].JSON;

                        var progressivePrediction = vw.Learn(json, VowpalWabbitPredictionType.ActionProbabilities);
                        // TODO: validate eval output
                    }

                    vw.Native.SaveModel("offline.json.2.model");
                }

                // download online model
                new CloudBlob(blobs.ModelBlob.Uri, blobs.BlobClient.Credentials).DownloadToFile("online.model", FileMode.Create);

                // validate that the model is the same
                CollectionAssert.AreEqual(
                    File.ReadAllBytes("offline.json.model"),
                    File.ReadAllBytes("online.model"),
                    "Offline and online model differs. Run to 'vw -i online.model --readable_model online.model.txt' to compare");
            }
        }

        [TestMethod]
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
            public string ModelId;

            public List<string> EventIds;
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

        private static SynchronizedCollection<ExceptionTelemetry> RegisterAppInsightExceptionHook()
        {
            var exceptions = new SynchronizedCollection<ExceptionTelemetry>();
            var builder = TelemetryConfiguration.Active.TelemetryProcessorChainBuilder;
            builder.Use((next) => new TestTelemetryProcessor(next, exceptions));
            builder.Build();

            return exceptions;
        }

        internal class ModelBlobs
        {
            internal CloudBlobClient BlobClient;
            internal CloudBlobContainer ModelContainer;
            internal CloudBlockBlob CurrentModel;
            internal CloudBlobContainer TrainerContainer;
            internal IListBlobItem ModelBlob;
            internal IListBlobItem ModelTrackbackBlob;
            internal IListBlobItem StateJsonBlob;

            internal ModelBlobs(string storageConnectionString)
            {
                this.BlobClient = CloudStorageAccount.Parse(storageConnectionString).CreateCloudBlobClient();

                this.ModelContainer = this.BlobClient.GetContainerReference("mwt-models");
                this.CurrentModel = this.ModelContainer.GetBlockBlobReference("current");
                this.TrainerContainer = this.BlobClient.GetContainerReference("onlinetrainer");
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

            internal async Task PollTrainerCheckpoint(SynchronizedCollection<ExceptionTelemetry> exceptions)
            {
                // wait for fiels to show up
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
                    this.ModelBlob = blobs.FirstOrDefault(b => b.Uri.ToString().EndsWith("model"));
                    this.ModelTrackbackBlob = blobs.FirstOrDefault(b => b.Uri.ToString().EndsWith("model.trackback"));
                    this.StateJsonBlob = blobs.FirstOrDefault(b => b.Uri.ToString().EndsWith("state.json"));

                    if (this.ModelBlob == null || this.ModelTrackbackBlob == null || this.StateJsonBlob == null)
                        continue;

                    return;
                }

                Assert.Fail("Trainer didn't produce checkpoints");
            }

            internal Trackback DownloadTrackback()
            {
                var trackbackStr = this.DownloadNonEmptyBlob(this.ModelTrackbackBlob);

                var trackback = trackbackStr.Split('\n');
                // modelid: faf5e313-46bb-4852-af05-576c3a1c2c67
                var m = Regex.Match(trackback[0], "^modelid: (.+)$");

                Assert.IsTrue(m.Success, $"Unable to extract model id from trackback file. Line '{trackback[0]}'");

                return new Trackback
                {
                    ModelId = m.Groups[1].Value,
                    EventIds = trackback.Skip(1).ToList()
                };
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

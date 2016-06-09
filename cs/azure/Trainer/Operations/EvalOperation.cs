using Microsoft.ApplicationInsights;
using Microsoft.ServiceBus.Messaging;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reactive.Linq;
using System.Reactive.Subjects;
using System.Text;
using System.Threading.Tasks;
using System.Threading.Tasks.Dataflow;
using VW;
using VW.Labels;

namespace VowpalWabbit.Azure.Trainer.Operations
{
    internal sealed class EvalData
    {
        internal string JSON { get; set; }

        internal string PolicyName { get; set; }
    }

    internal sealed class EvalOperation : IDisposable
    {
        private readonly EventHubClient evalEventHubClient;

        private TransformManyBlock<object, EvalData> evalBlock;
        private IDisposable evalBlockDisposable;
        private TelemetryClient telemetry;

        internal EvalOperation(OnlineTrainerSettingsInternal settings) 
        {
            this.telemetry = new TelemetryClient();

            // evaluation pipeline 
            this.evalEventHubClient = EventHubClient.CreateFromConnectionString(settings.EvalEventHubConnectionString);

            this.evalBlock = new TransformManyBlock<object, EvalData>(
                (Func<object, IEnumerable<EvalData>>)this.OfflineEvaluate,
                new ExecutionDataflowBlockOptions
                {
                    MaxDegreeOfParallelism = 4,
                    BoundedCapacity = 1024
                });

            this.evalBlock.Completion.ContinueWith(t =>
            {
                this.telemetry.TrackTrace($"Stage 3 - Evaluation pipeline completed: {t.Status}");
                if (t.IsFaulted)
                    this.telemetry.TrackException(t.Exception);
            });

            // batch output together to match EventHub throughput by maintaining maximum latency of 5 seconds
            this.evalBlockDisposable = this.evalBlock.AsObservable()
                .GroupBy(k => k.PolicyName)
                   .Select(g =>
                        g.Window(TimeSpan.FromSeconds(5))
                         .Select(w => w.Buffer(245 * 1024, e => Encoding.UTF8.GetByteCount(e.JSON)))
                         .SelectMany(w => w)
                         .Subscribe(this.UploadEvaluation))
                   .Publish()
                   .Connect();
        }

        internal ITargetBlock<object> TargetBlock { get { return this.evalBlock; } }

        private static EvalData Create(ContextualBanditLabel label, string policyName, uint actionTaken)
        {
            return new EvalData
            {
                PolicyName = policyName,
                JSON = JsonConvert.SerializeObject(
                    new
                    {
                        name = policyName,
                        cost = VowpalWabbitContextualBanditUtil.GetUnbiasedCost(label.Action, actionTaken, label.Action, label.Probability)
                    })
            };
        }

        private IEnumerable<EvalData> OfflineEvaluate(object trainerResult)
        {
            var tr = trainerResult as TrainerResult;
            if (tr == null)
            {
                this.telemetry.TrackTrace($"Received invalid data: {trainerResult}");
                yield break;
            }

            yield return Create(tr.Label, "Latest Policy", tr.LearnedAction);
            yield return Create(tr.Label, "Deployed Policy", tr.Label.Action);

            for (int action = 1; action <= tr.NumberOfActions; action++)
                yield return Create(tr.Label, $"Constant Policy {action}", (uint)action);
        }

        private void UploadEvaluation(IList<EvalData> batch)
        {
            try
            {
                var eventData = new EventData(Encoding.UTF8.GetBytes(string.Join("\n", batch.Select(b => b.JSON))))
                {
                    PartitionKey = batch.First().PolicyName
                };

                this.evalEventHubClient.Send(eventData);
            }
            catch (Exception e)
            {
                this.telemetry.TrackException(e);
            }
        }

        public void Dispose()
        {
            if (this.evalBlock != null)
            {
                this.evalBlock.Complete();
                this.evalBlock.Completion.Wait(TimeSpan.FromMinutes(1));
                this.evalBlock = null;
            }

            if (this.evalBlockDisposable != null)
            {
                this.evalBlockDisposable.Dispose();
                this.evalBlockDisposable = null;
            }
        }
    }
}

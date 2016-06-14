// --------------------------------------------------------------------------------------------------------------------
// <copyright file="TrainEventProcessorFactory.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using Microsoft.ApplicationInsights;
using Microsoft.ApplicationInsights.DataContracts;
using Microsoft.ServiceBus.Messaging;
using System;
using System.Linq;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using System.Threading.Tasks.Dataflow;
using VW;
using Newtonsoft.Json;
using System.IO;
using System.Diagnostics;
using System.Reactive.Linq;
using System.Reactive.Subjects;
using VW.Labels;
using System.Text;
using VowpalWabbit.Azure.Trainer;
using VowpalWabbit.Azure.Trainer.Operations;
using VW.Serializer;
using VowpalWabbit.Azure.Trainer.Data;

namespace VowpalWabbit.Azure
{
    internal sealed class TrainEventProcessorFactory : IEventProcessorFactory, IDisposable
    {
        private readonly TelemetryClient telemetry;
        private readonly PerformanceCounters performanceCounters;
        private readonly Learner trainer;

        private EvalOperation evalOperation;
        //private LatencyOperation latencyOperation;

        private TransformManyBlock<PipelineData, PipelineData> deserializeBlock;
        private TransformManyBlock<object, object> learnBlock;
        private ActionBlock<object> checkpointBlock;
        private IDisposable checkpointTrigger;

        internal TrainEventProcessorFactory(OnlineTrainerSettingsInternal settings, Learner trainer, PerformanceCounters performanceCounters)
        {
            if (settings == null)
                throw new ArgumentNullException(nameof(settings));

            if (trainer == null)
                throw new ArgumentNullException(nameof(trainer));

            if (performanceCounters == null)
                throw new ArgumentNullException(nameof(performanceCounters));

            this.trainer = trainer;
            this.performanceCounters = performanceCounters;

            this.telemetry = new TelemetryClient();
            this.telemetry.Context.Component.Version = GetType().Assembly.GetName().Version.ToString();

            this.evalOperation = new EvalOperation(settings);
            //this.latencyOperation = new LatencyOperation();

            this.deserializeBlock = new TransformManyBlock<PipelineData, PipelineData>(
                (Func<PipelineData, IEnumerable<PipelineData>>)this.Stage1_Deserialize,
                new ExecutionDataflowBlockOptions
                {
                    MaxDegreeOfParallelism = 4, // Math.Max(2, Environment.ProcessorCount - 1),
                    BoundedCapacity = 1024
                });
            this.deserializeBlock.Completion.Trace(this.telemetry, "Stage 1 - Deserialization");

            this.learnBlock = new TransformManyBlock<object, object>(
                (Func<object, IEnumerable<object>>)this.Stage2_ProcessEvent,
                new ExecutionDataflowBlockOptions
                {
                    MaxDegreeOfParallelism = 1,
                    BoundedCapacity = 1024
                });
            this.learnBlock.Completion.Trace(this.telemetry, "Stage 2 - Learning");

            // trigger checkpoint checking every second
            this.checkpointTrigger = Observable.Interval(TimeSpan.FromSeconds(1))
                .Select(_ => new CheckpointTriggerEvent())
                .Subscribe(this.learnBlock.AsObserver());

            this.checkpointBlock = new ActionBlock<object>(
                this.trainer.Checkpoint,
                new ExecutionDataflowBlockOptions
                {
                    MaxDegreeOfParallelism = 1,
                    BoundedCapacity = 4
                });
            this.learnBlock.Completion.Trace(this.telemetry, "Stage 3 - CheckPointing");

            // setup pipeline
            this.deserializeBlock.LinkTo(
                this.learnBlock,
                new DataflowLinkOptions { PropagateCompletion = true });

            this.learnBlock.LinkTo(
                this.evalOperation.TargetBlock,
                new DataflowLinkOptions { PropagateCompletion = true },
                obj => obj is TrainerResult);

            this.learnBlock.LinkTo(
                this.checkpointBlock,
                new DataflowLinkOptions { PropagateCompletion = true },
                obj => obj is CheckpointData);

            // consume all unmatched
            this.learnBlock.LinkTo(DataflowBlock.NullTarget<object>());
        }

        internal void UpdatePerformanceCounters()
        {
            this.performanceCounters.Stage1_JSON_Queue.RawValue = this.deserializeBlock.InputCount;
            this.performanceCounters.Stage2_Learn_Queue.RawValue = this.learnBlock.InputCount;
            this.performanceCounters.Stage3_Checkpoint_Queue.RawValue = this.checkpointBlock.InputCount;
        }

        internal async Task Stage0_Split(PartitionContext context, IEnumerable<EventData> messages)
        {
            foreach (EventData eventData in messages)
            {
                try
                {
                    using (var eventStream = eventData.GetBodyStream())
                    {
                        using (var sr = new StreamReader(eventStream, Encoding.UTF8))
                        {
                            string line;
                            while ((line = await sr.ReadLineAsync()) != null)
                            {
                                var data = new PipelineData
                                {
                                    JSON = line,
                                    PartitionKey = context.Lease.PartitionId,
                                    Offset = eventData.Offset
                                };

                                // TODO: ArrayBuffer to avoid string allocation...
                                // also just send char ref + offset + length
                                if (!await this.deserializeBlock.SendAsync(data))
                                    this.telemetry.TrackTrace("Failed to enqueue data");
                            }

                            this.performanceCounters.Stage0_IncomingBytesPerSec.IncrementBy(eventStream.Position);
                            this.performanceCounters.Stage0_Batches_Size.IncrementBy(eventStream.Position);
                            this.performanceCounters.Stage0_Batches_SizeBase.Increment();
                        }
                    }

                    this.performanceCounters.Stage0_BatchesPerSec.Increment();
                    this.performanceCounters.Stage0_Batches_Total.Increment();
                }
                catch (Exception)
                {

                    throw;
                }
            }
        }

        private IEnumerable<PipelineData> Stage1_Deserialize(PipelineData data)
        {
            try
            {
                using (var jsonReader = new JsonTextReader(new StringReader(data.JSON)))
                {
                    //jsonReader.FloatParser = Util.ReadDoubleString;
                    // jsonReader.ArrayPool = pool;

                    VowpalWabbitJsonSerializer vwJsonSerializer = null;
                    try
                    {
                        vwJsonSerializer = new VowpalWabbitJsonSerializer(this.trainer.VowpalWabbit, this.trainer.ReferenceResolver);

                        vwJsonSerializer.RegisterExtension((state, property) =>
                        {
                            if (property.Equals("_eventid", StringComparison.OrdinalIgnoreCase))
                            {
                                if (!state.Reader.Read() && state.Reader.TokenType != JsonToken.String)
                                    throw new VowpalWabbitJsonException(state.Reader, "Expected string");
                                data.EventId = (string)state.Reader.Value;

                                return true;
                            }

                            return false;
                        });

                        data.Example = vwJsonSerializer.ParseAndCreate(jsonReader);

                        if (data.Example == null)
                        {
                            // unable to create example due to missing data
                            // will be trigger later
                            vwJsonSerializer.UserContext = data.Example;
                            // make sure the serialize is not deallocated
                            vwJsonSerializer = null;
                        }
                    }
                    finally
                    {
                        if (vwJsonSerializer != null)
                            vwJsonSerializer.Dispose();
                    }

                    performanceCounters.Stage1_JSON_DeserializePerSec.Increment();

                    // delayed
                    if (data.Example == null)
                    {
                        this.performanceCounters.Feature_Requests_Pending.Increment();
                        yield break;
                    }
                }
            }
            catch (Exception ex)
            {
                this.telemetry.TrackException(ex, new Dictionary<string, string> { { "JSON", data.JSON } });

                this.performanceCounters.Stage2_Faulty_Examples_Total.Increment();
                this.performanceCounters.Stage2_Faulty_ExamplesPerSec.Increment();

                yield break;
            }

            yield return data;
        }

        private IEnumerable<object> Stage2_ProcessEvent(object evt)
        {
            // single threaded loop
            var eventHubExample = evt as PipelineData;
            if (eventHubExample != null)
            {
                yield return this.trainer.Learn(eventHubExample);

                // this.latencyOperation.Process(result);

                if (this.trainer.ShouldCheckpoint(1))
                    yield return this.trainer.CreateCheckpointData(updateClientModel: true);
            }
            else if (evt is CheckpointEvent)
                yield return this.trainer.CreateCheckpointData(updateClientModel: false);
            else if (evt is CheckpointTriggerEvent)
            {
                if (this.trainer.ShouldCheckpoint(0))
                    yield return this.trainer.CreateCheckpointData(updateClientModel: true);
            }
            else
                this.telemetry.TrackTrace($"Unsupported stage 2 event '{evt}'", SeverityLevel.Warning);
        }

        public IEventProcessor CreateEventProcessor(PartitionContext context)
        {
            return new LearnEventProcessor(this, this.performanceCounters);
        }

        public ITargetBlock<object> LearnBlock { get { return this.learnBlock; } }

        /// <summary>
        /// Performs application-defined tasks associated with freeing, releasing, or resetting unmanaged resources.
        /// </summary>
        public void Dispose()
        {
            if (this.checkpointTrigger != null)
            {
                this.checkpointTrigger.Dispose();
                this.checkpointTrigger = null;
            }

            if (this.learnBlock != null)
            {
                // complete beginning of the pipeline
                this.deserializeBlock.Complete();

                // wait at the end of the pipeline
                this.checkpointBlock.Completion.Wait(TimeSpan.FromMinutes(1));
            }

            if (this.evalOperation != null)
            {
                this.evalOperation.Dispose();
                this.evalOperation = null;
            }

            //if (this.latencyOperation != null)
            //{
            //    this.latencyOperation.Dispose();
            //    this.latencyOperation = null;
            //}
        }
    }
}

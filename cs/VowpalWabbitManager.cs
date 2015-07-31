using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Threading.Tasks.Dataflow;

namespace VW
{
    public class VowpalWabbitManager : IDisposable
    {
        internal VowpalWabbitThreaded[] vws;

        internal readonly ActionBlock<Action<VowpalWabbitThreaded>>[] actionBlocks;

        private readonly IObserver<Action<VowpalWabbitThreaded>>[] observers;

        /// <summary>
        /// Invoked right after the root node performed AllReduce with the other instances.
        /// </summary>
        private readonly ConcurrentList<Action<VowpalWabbitNative>> syncActions;

        private Task[] completionTasks;

        private int exampleCount;

        private readonly Random random = new Random(42);

        public VowpalWabbitManager(VowpalWabbitSettings settings)
        {
            if (settings.ParallelOptions == null)
            {
                throw new ArgumentException("settings.ParallelOptions must be set");
            }

            this.Settings = settings;

            if (this.Settings.CancellationToken == null)
            {
                this.Settings.CancellationToken = new CancellationToken();
            }

            this.exampleCount = 0;
            this.syncActions = new ConcurrentList<Action<VowpalWabbitNative>>();

            this.vws = new VowpalWabbitThreaded[settings.ParallelOptions];
            this.actionBlocks = new ActionBlock<Action<VowpalWabbitThreaded>>[settings.ParallelOptions];
            this.observers = new IObserver<Action<VowpalWabbitThreaded>>[settings.ParallelOptions];

            // setup AllReduce chain
            // root closure
            {
                var vw = this.vws[0] = new VowpalWabbitThreaded(settings, 0);
                uint unitCount = 0; // unitCount <= example count due to multiline examples!

                var actionBlock = this.actionBlocks[0] = new ActionBlock<Action<VowpalWabbitThreaded>>(
                    action => action(vw),
                    new ExecutionDataflowBlockOptions
                    {
                        MaxDegreeOfParallelism = 1,
                        TaskScheduler = settings.ParallelOptions.TaskScheduler,
                        CancellationToken = settings.ParallelOptions.CancellationToken,
                        BoundedCapacity = settings.MaxExampleQueueLengthPerInstance
                    });
            }

            for (int i = 1; i < settings.ParallelOptions; i++)
            {
                // closure vars
                var vw = new VowpalWabbitThreaded(settings, this.vws[i - 1], i);
                uint unitCount = 0; // unitCount <= example count due to multiline examples!

                var actionBlock = this.actionBlocks[i] = new ActionBlock<Action<VowpalWabbitThreaded>>(
                    action => action(vw),
                    new ExecutionDataflowBlockOptions
                    {
                        MaxDegreeOfParallelism = 1,
                        TaskScheduler = settings.ParallelOptions.TaskScheduler,
                        CancellationToken = settings.ParallelOptions.CancellationToken,
                        BoundedCapacity = settings.MaxExampleQueueLengthPerInstance
                    });
            }

            // get observers to allow for blocking calls
            this.observers = this.actionBlocks.Select(ab => ab.AsObserver()).ToArray();

            this.completionTasks = new Task[settings.ParallelOptions.MaxDegreeOfParallelism];
            // root closure
            {
                var vw = this.vws[0];
                this.completionTasks[0] = this.actionBlocks[0].Completion
                    .ContinueWith(() =>
                    {
                        // perform final AllReduce
                        vw.EndOfPass();

                        // execute synchronization actions
                        foreach (var syncAction in this.syncActions.RemoveAll())
                        {
                            syncAction(vw);
                        }
                    })
                    .ConfigureAwait(false);
            }

            for (int i = 1; i < this.vws.Length; i++)
            {
                // perform final AllReduce
                var vw = this.vws[i];
                this.completionTasks[i] = this.actionBlocks[i].Completion
                    .ContinueWith(() => vw.EndOfPass(), this.Settings.ParallelOptions.CancellationToken)
                    .ConfigureAwait(false);
            }
        }

        public VowpalWabbitAsync<TExample> Create<TExample>()
        {
            return new VowpalWabbitAsync<TExample>(this, this.vws);
        }

        public VowpalWabbit<TExample, TActionDependentFeature> Create<TExample, TActionDependentFeature>()
        {
            return new VowpalWabbitAsync<TExample, TActionDependentFeature>(this, this.vws);
        }

        private void CheckEndOfPass()
        {
            // get next node using round-robin
            var exampleCount = (uint)Interlocked.Increment(ref this.exampleCount);
            
            if (exampleCount % this.Settings.ExampleCountPerRun.Length == 0)
            {
                this.observers[0].OnNext(vw =>
                {
                    // perform AllReduce
                    vw.EndOfPass();

                    // execute synchronization actions
                    foreach (var syncAction in this.syncActions.RemoveAll())
                    {
                        syncAction(vw);
                    }
                });

                for (int i = 1; i < this.observers.Length; i++)
                {
                    // perform AllReduce
                    this.observers[0].OnNext(vw => vw.EndOfPass());
                }
            }

        }

        public void Post(Action<VowpalWabbitThreaded> action)
        {
            this.CheckEndOfPass();

            // randomly distribute data
            var index = random.Next(this.observers.Length);
            this.observers[index].OnNext(action);
        }
        
        internal Task<T> Post<T>(Func<VowpalWabbitThreaded, T> func)
        {
            this.CheckEndOfPass();

            var completionSource = new TaskCompletionSource<T>();

            // randomly distribute data
            var index = random.Next(this.observers.Length);
            this.observers[index].OnNext(vw => 
            {
                try 
	            {
                    completionSource.SetResult(func(vw));
	            }
	            catch (Exception ex)
	            {
                    completionSource.SetException(ex);
	            }
            });

            return completionSource.Task;
        }

        public void Learn(string line)
        {
            this.Post(vw => vw.Learn(line));
        }

        public Task<VowpalWabbitPerformanceStatistics> PerformanceStatistics
        {
            get 
            {
                var completionSource = new TaskCompletionSource<void>();

                this.syncActions.Add(vw => completionSource.SetResult(vw.PerformanceStatistics));

                return completionSource.Task;
            }
        }

        public Task Complete()
        {
            // make sure no more sync actions are added, which might otherwise never been called
            this.syncActions.CompleteAdding();

            foreach (var actionBlock in this.actionBlocks)
            {
                actionBlock.Complete();
            }
        }

        public async Task SaveModel()
        {
            var completionSource = new TaskCompletionSource<void>();

            this.syncActions.Add(vw => 
            {
                vw.SaveModel();
                completionSource.SetResult();
            });

            return completionSource.Task;
        }

        public async Task SaveModel(string filename)
        {
            var completionSource = new TaskCompletionSource<void>();

            this.syncActions.Add(vw => 
            {
                vw.SaveModel();
                completionSource.SetResult();
            });

            return completionSource.Task;
        }

        public VowpalWabbitSettings Settings
        {
            get;
            private set;
        }

        /// <summary>
        /// Performs application-defined tasks associated with freeing, releasing, or resetting unmanaged resources.
        /// </summary>
        public void Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        private void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (this.completionTasks != null)
                {
                    // mark completion
                    this.Complete()
                        .Wait(this.Settings.ParallelOptions.CancellationToken)

                    // wait for all actionblocks to finish
                    Task.WhenAll(this.completionTasks)
                        .Wait(this.Settings.ParallelOptions.CancellationToken);

                    this.completionTasks = null;
                }

                if (this.vws != null)
                {
                    foreach (var vw in this.vws)
                    {
                        vw.Dispose();
                    }

                    this.vws = null;
                }
            }
        }

        private class ConcurrentList<T>
        {
            private bool completed = false;

            private readonly List<T> items = new List<T>();

            private readonly object lockObject = new object();

            public void Add(T item)
            {
                lock (this.lockObject)
                {
                    if (completed)
                    {
                        throw new InvalidOperationException("ConcurrentList has been marked completed.");
                    }

                    this.items.Add(item);
                }
            }

            public void CompleteAdding()
            {
                lock (this.lockObject)
                {
                    this.completed = true;
                }
            }

            public T[] RemoveAll()
            {
                lock (this.lockObject)
                {
                    return this.items.ToArray();
                }
            }
        }
    }
}

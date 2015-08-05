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
    public class VowpalWabbitThreadedLearning : IDisposable
    {
        internal VowpalWabbit[] vws;

        internal readonly ActionBlock<Action<VowpalWabbit>>[] actionBlocks;

        private readonly IObserver<Action<VowpalWabbit>>[] observers;

        /// <summary>
        /// Invoked right after the root node performed AllReduce with the other instances.
        /// </summary>
        private readonly ConcurrentList<Action<VowpalWabbit>> syncActions;

        private Task[] completionTasks;

        private int exampleCount;

        private readonly Random random = new Random(42);

        private readonly Func<uint, int> exampleDistributor;

        public VowpalWabbitThreadedLearning(VowpalWabbitSettings settings)
        {
            if (settings.ParallelOptions == null)
            {
                throw new ArgumentException("settings.ParallelOptions must be set");
            }

            this.Settings = settings;

            if (this.Settings.ParallelOptions.CancellationToken == null)
            {
                this.Settings.ParallelOptions.CancellationToken = new CancellationToken();
            }

            switch (this.Settings.ExampleDistribution)
            {
                case VowpalWabbitExampleDistribution.UniformRandom:
                    this.exampleDistributor = _ => this.random.Next(this.observers.Length);
                    break;
                case VowpalWabbitExampleDistribution.RoundRobin:
                    this.exampleDistributor = localExampleCount => (int)(localExampleCount % this.observers.Length);
                    break;
            }

            this.exampleCount = 0;
            this.syncActions = new ConcurrentList<Action<VowpalWabbit>>();

            this.vws = new VowpalWabbit[settings.ParallelOptions.MaxDegreeOfParallelism];
            this.actionBlocks = new ActionBlock<Action<VowpalWabbit>>[settings.ParallelOptions.MaxDegreeOfParallelism];
            this.observers = new IObserver<Action<VowpalWabbit>>[settings.ParallelOptions.MaxDegreeOfParallelism];

            // setup AllReduce chain
            // root closure
            {
                var vw = this.vws[0] = new VowpalWabbit(settings.ShallowCopy(node: 0));
                uint unitCount = 0; // unitCount <= example count due to multiline examples!

                var actionBlock = this.actionBlocks[0] = new ActionBlock<Action<VowpalWabbit>>(
                    action => action(vw),
                    new ExecutionDataflowBlockOptions
                    {
                        MaxDegreeOfParallelism = 1,
                        TaskScheduler = settings.ParallelOptions.TaskScheduler,
                        CancellationToken = settings.ParallelOptions.CancellationToken,
                        BoundedCapacity = (int)settings.MaxExampleQueueLengthPerInstance
                    });
            }

            var childSettings = settings.ShallowCopy(root: this.vws[0]);
            for (int i = 1; i < settings.ParallelOptions.MaxDegreeOfParallelism; i++)
            {
                // closure vars
                var vw = new VowpalWabbit(childSettings.ShallowCopy(node: (uint)i));
                uint unitCount = 0; // unitCount <= example count due to multiline examples!

                var actionBlock = this.actionBlocks[i] = new ActionBlock<Action<VowpalWabbit>>(
                    action => action(vw),
                    new ExecutionDataflowBlockOptions
                    {
                        MaxDegreeOfParallelism = 1,
                        TaskScheduler = settings.ParallelOptions.TaskScheduler,
                        CancellationToken = settings.ParallelOptions.CancellationToken,
                        BoundedCapacity = (int)settings.MaxExampleQueueLengthPerInstance
                    });
            }

            // get observers to allow for blocking calls
            this.observers = this.actionBlocks.Select(ab => ab.AsObserver()).ToArray();

            this.completionTasks = new Task[settings.ParallelOptions.MaxDegreeOfParallelism];
            // root closure
            {
                var vw = this.vws[0];
                this.completionTasks[0] = this.actionBlocks[0].Completion
                    .ContinueWith(_ =>
                    {
                        // perform final AllReduce
                        vw.EndOfPass();

                        // execute synchronization actions
                        foreach (var syncAction in this.syncActions.RemoveAll())
                        {
                            syncAction(vw);
                        }
                    });
            }

            for (int i = 1; i < this.vws.Length; i++)
            {
                // perform final AllReduce
                var vw = this.vws[i];
                this.completionTasks[i] = this.actionBlocks[i].Completion
                    .ContinueWith(_ => vw.EndOfPass(), this.Settings.ParallelOptions.CancellationToken);
            }
        }

        public VowpalWabbitAsync<TExample> Create<TExample>()
        {
            return new VowpalWabbitAsync<TExample>(this);
        }

        public VowpalWabbitAsync<TExample, TActionDependentFeature> Create<TExample, TActionDependentFeature>()
        {
            return new VowpalWabbitAsync<TExample, TActionDependentFeature>(this);
        }

        private uint CheckEndOfPass()
        {
            var exampleCount = (uint)Interlocked.Increment(ref this.exampleCount);
            
            if (exampleCount % this.Settings.ExampleCountPerRun == 0)
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
                    this.observers[i].OnNext(vw => vw.EndOfPass());
                }
            }

            return exampleCount;
        }

        public void Post(Action<VowpalWabbit> action)
        {
            var exampleCount = this.CheckEndOfPass();

            // dispatch
            this.observers[this.exampleDistributor(exampleCount)].OnNext(action);
        }

        internal Task<T> Post<T>(Func<VowpalWabbit, T> func)
        {
            var exampleCount = this.CheckEndOfPass();

            var completionSource = new TaskCompletionSource<T>();

            // dispatch
            this.observers[this.exampleDistributor(exampleCount)].OnNext(vw => 
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
            this.Post(vw => 
            {
                using (var builder = new VowpalWabbitExampleBuilder(vw))
                {
                    using (var ex = vw.ParseLine(line))
                    {
                        vw.Learn(ex);
                    }
                }
            });
        }

        public Task<VowpalWabbitPerformanceStatistics> PerformanceStatistics
        {
            get 
            {
                var completionSource = new TaskCompletionSource<VowpalWabbitPerformanceStatistics>();

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

            return Task.WhenAll(this.actionBlocks.Select(a => a.Completion));

        }

        public Task SaveModel()
        {
            var completionSource = new TaskCompletionSource<bool>();

            this.syncActions.Add(vw => 
            {
                vw.SaveModel();
                completionSource.SetResult(true);
            });

            return completionSource.Task;
        }

        public Task SaveModel(string filename)
        {
            var completionSource = new TaskCompletionSource<bool>();

            this.syncActions.Add(vw => 
            {
                vw.SaveModel();
                completionSource.SetResult(true);
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
                        .Wait(this.Settings.ParallelOptions.CancellationToken);

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

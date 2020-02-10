// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitThreadedLearning.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using System.Threading.Tasks.Dataflow;

namespace VW
{
    /// <summary>
    /// VW wrapper supporting multi-core learning by utilizing thread-based allreduce.
    /// </summary>
    public class VowpalWabbitThreadedLearning : IDisposable
    {
        /// <summary>
        /// Random generator used by uniform random example distributor.
        /// </summary>
        /// <remarks>Initialized with static seed to enable reproducability.</remarks>
        private readonly Random random = new Random(42);

        /// <summary>
        /// Configurable example distribution function choosing the vw instance for the next example.
        /// </summary>
        private readonly Func<uint, int> exampleDistributor;

        /// <summary>
        /// Native vw instances setup for thread-based allreduce
        /// </summary>
        private VowpalWabbit[] vws;

        /// <summary>
        /// Worker threads with a nice message queue infront that will start blocking once it's too full.
        /// </summary>
        private readonly ActionBlock<Action<VowpalWabbit>>[] actionBlocks;

        /// <summary>
        /// The <see cref="actionBlocks"/> only offer non-blocking methods. Getting observers and calling OnNext() enables
        /// blocking once the queue is full.
        /// </summary>
        private readonly IObserver<Action<VowpalWabbit>>[] observers;

        /// <summary>
        /// Invoked right after the root node performed AllReduce with the other instances.
        /// </summary>
        private readonly ConcurrentList<Action<VowpalWabbit>> syncActions;

        /// <summary>
        /// Task enable waiting for clients on completion after all action blocks have finished (incl. cleanup).
        /// </summary>
        private Task[] completionTasks;

        /// <summary>
        /// Number of examples seen sofar. Used by round robin example distributor.
        /// </summary>
        private int exampleCount;

        /// <summary>
        /// Initializes a new instance of the <see cref="VowpalWabbitThreadedLearning"/> class.
        /// </summary>
        /// <param name="settings">Common settings used for vw instances.</param>
        public VowpalWabbitThreadedLearning(VowpalWabbitSettings settings)
        {
            if (settings == null)
                throw new ArgumentNullException("settings");

            if (settings.ParallelOptions == null)
                throw new ArgumentNullException("settings.ParallelOptions must be set");
            Contract.EndContractBlock();

            this.Settings = settings;

            if (this.Settings.ParallelOptions.CancellationToken == null)
                this.Settings.ParallelOptions.CancellationToken = new CancellationToken();

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
                var nodeSettings = (VowpalWabbitSettings)settings.Clone();
                nodeSettings.Node = 0;
                var vw = this.vws[0] = new VowpalWabbit(nodeSettings);

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

            for (int i = 1; i < settings.ParallelOptions.MaxDegreeOfParallelism; i++)
            {
                // closure vars
                var nodeSettings = (VowpalWabbitSettings)settings.Clone();
                nodeSettings.Root = this.vws[0];
                nodeSettings.Node = (uint)i;
                var vw = this.vws[i] = new VowpalWabbit(nodeSettings);

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

        /// <summary>
        /// VowpalWabbit instances participating in AllReduce.
        /// </summary>
        public VowpalWabbit[] VowpalWabbits
        {
            get { return this.vws; }
        }

        /// <summary>
        /// Creates a new instance of <see cref="VowpalWabbitAsync{TExample}"/> to feed examples of type <typeparamref name="TExample"/>.
        /// </summary>
        /// <typeparam name="TExample">The user example type.</typeparam>
        /// <returns>A new instance of <see cref="VowpalWabbitAsync{TExample}"/>.</returns>
        public VowpalWabbitAsync<TExample> Create<TExample>()
        {
            return new VowpalWabbitAsync<TExample>(this);
        }

        /// <summary>
        /// Creates a new instance of <see cref="VowpalWabbitAsync{TExample,TActionDependentFeature}"/> to feed multi-line
        /// examples of type <typeparamref name="TExample"/> and <typeparamref name="TActionDependentFeature"/>.
        /// </summary>
        /// <typeparam name="TExample">The user example type.</typeparam>
        /// <typeparam name="TActionDependentFeature">The user action dependent feature type.</typeparam>
        /// <returns>A new instance of <see cref="VowpalWabbitAsync{TExample,TActionDependentFeature}"/>.</returns>
        public VowpalWabbitAsync<TExample, TActionDependentFeature> Create<TExample, TActionDependentFeature>()
        {
            return new VowpalWabbitAsync<TExample, TActionDependentFeature>(this);
        }

        /// <summary>
        /// Everytime <see cref="VowpalWabbitSettings.ExampleCountPerRun"/> examples have been enqueued,
        /// an AllReduce-sync operation (<see cref="VowpalWabbit.EndOfPass"/>) is injected.
        /// </summary>
        /// <returns>The number of examples enqueued so far.</returns>
        private uint CheckEndOfPass()
        {
            var exampleCount = (uint)Interlocked.Increment(ref this.exampleCount);

            // since there is no lock the input queue, it's not guaranteed that exactly
            // that number of examples are processed (but maybe a few more).
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

        /// <summary>
        /// Enqueues an action to be executed on one of vw instances.
        /// </summary>
        /// <param name="action">The action to be executed (e.g. Learn/Predict/...).</param>
        /// <remarks>If number of actions waiting to be executed has reached <see name="VowpalWabbitSettings.MaxExampleQueueLengthPerInstance"/> this method blocks.</remarks>
        public void Post(Action<VowpalWabbit> action)
        {
            Contract.Requires(action != null);

            var exampleCount = this.CheckEndOfPass();

            // dispatch
            this.observers[this.exampleDistributor(exampleCount)].OnNext(action);
        }

        /// <summary>
        /// Enqueues a task to be executed by single VowpalWabbit instance.
        /// </summary>
        /// <remarks>Which VowpalWabbit instance chosen, is determined by <see cref="VowpalWabbitSettings.ExampleDistribution"/>.</remarks>
        /// <typeparam name="T">The return type of the task.</typeparam>
        /// <param name="func">The task to be executed.</param>
        /// <returns>The awaitable result of the supplied task.</returns>
        internal Task<T> Post<T>(Func<VowpalWabbit, T> func)
        {
            Contract.Requires(func!= null);

            var exampleCount = this.CheckEndOfPass();

            var completionSource = new TaskCompletionSource<T>();

            // dispatch to a Vowpal Wabbit instance
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

        /// <summary>
        /// Learns from the given example.
        /// </summary>
        /// <param name="line">The example to learn.</param>
        public void Learn(string line)
        {
            Contract.Requires(line != null);

            this.Post(vw => vw.Learn(line));
        }

        /// <summary>
        /// Learns from the given example.
        /// </summary>
        /// <param name="lines">The multi-line example to learn.</param>
        public void Learn(IEnumerable<string> lines)
        {
            Contract.Requires(lines != null);

            this.Post(vw => vw.Learn(lines));
        }

        /// <summary>
        /// Synchronized performance statistics.
        /// </summary>
        /// <remarks>The task is only completed after synchronization of all instances, triggered <see cref="VowpalWabbitSettings.ExampleCountPerRun"/> example.</remarks>
        public Task<VowpalWabbitPerformanceStatistics> PerformanceStatistics
        {
            get
            {
                var completionSource = new TaskCompletionSource<VowpalWabbitPerformanceStatistics>();

                this.syncActions.Add(vw => completionSource.SetResult(vw.PerformanceStatistics));

                return completionSource.Task;
            }
        }

        /// <summary>
        /// Signal that no more examples are send.
        /// </summary>
        /// <returns>Task completes once the learning and cleanup is done.</returns>
        public Task Complete()
        {
            // make sure no more sync actions are added, which might otherwise never been called
            this.syncActions.CompleteAdding();

            foreach (var actionBlock in this.actionBlocks)
            {
                actionBlock.Complete();
            }

            return Task.WhenAll(this.completionTasks);

        }

        /// <summary>
        /// Saves a model as part of the synchronization.
        /// </summary>
        /// <returns>Task compeletes once the model is saved.</returns>
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

        /// <summary>
        /// Saves a model as part of the synchronization.
        /// </summary>
        /// <returns>Task compeletes once the model is saved.</returns>
        public Task SaveModel(string filename)
        {
            Contract.Requires(!string.IsNullOrEmpty(filename));

            var completionSource = new TaskCompletionSource<bool>();

            this.syncActions.Add(vw =>
            {
                vw.SaveModel(filename);
                completionSource.SetResult(true);
            });

            return completionSource.Task;
        }

        /// <summary>
        /// The settings shared across all instances.
        /// </summary>
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

        /// <summary>
        /// Thread-safe list implementation supporting completion.
        /// </summary>
        /// <typeparam name="T">The element type.</typeparam>
        private class ConcurrentList<T>
        {
            private bool completed = false;

            private readonly List<T> items = new List<T>();

            private readonly object lockObject = new object();

            /// <summary>
            ///  Adds an object to the end of the list.
            /// </summary>
            /// <param name="item">The object to be added to the list.</param>
            /// <remarks>Throws an <see cref="InvalidOperationException"/> if the <see cref="CompleteAdding"/> as called previously.</remarks>
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

            /// <summary>
            /// Marks this list as complete. Any subsequent calls to <see cref="Add"/> will trigger an <see cref="InvalidOperationException"/>.
            /// </summary>
            public void CompleteAdding()
            {
                lock (this.lockObject)
                {
                    this.completed = true;
                }
            }

            /// <summary>
            /// Removes all elements from the list.
            /// </summary>
            /// <returns>The elements removed.</returns>
            public T[] RemoveAll()
            {
                lock (this.lockObject)
                {
                    var ret =  this.items.ToArray();
                    this.items.Clear();

                    return ret;
                }
            }
        }
    }
}

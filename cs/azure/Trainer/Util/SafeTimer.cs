// --------------------------------------------------------------------------------------------------------------------
// <copyright file="SafeTimer.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Threading;
using System.Threading.Tasks;

namespace VW.Azure.Trainer
{
    internal class SafeTimer
    {
        private CancellationTokenSource cancellationTokenSource;
        private ManualResetEventSlim finishedEvent;

        internal SafeTimer(TimeSpan delay, Action action)
        {
            this.cancellationTokenSource = new CancellationTokenSource();
            var cancellationToken = cancellationTokenSource.Token;
            finishedEvent = new ManualResetEventSlim();

            Task.Factory
                .StartNew(async () =>
                    {
                        while (true)
                        {
                            cancellationToken.ThrowIfCancellationRequested();
                            action();
                            cancellationToken.ThrowIfCancellationRequested();
                            await Task.Delay(delay, cancellationToken);
                        }
                    }, 
                    TaskCreationOptions.LongRunning)
                .ContinueWith(t => finishedEvent.Set());
        }

        public void Stop(TimeSpan timeout)
        {
            cancellationTokenSource.Cancel();
            finishedEvent.Wait(timeout);
        }
    }
}

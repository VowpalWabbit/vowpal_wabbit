using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace VowpalWabbit.Azure.Trainer
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

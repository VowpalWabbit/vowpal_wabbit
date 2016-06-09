using Microsoft.ApplicationInsights;
using System;
using System.Linq;
using System.Reactive.Linq;
using System.Reactive.Subjects;
using System.Threading.Tasks;

namespace VowpalWabbit.Azure.Trainer
{
    internal abstract class ThrottledOperation<T> : IDisposable
    {
        private Subject<T> pipeline;

        private IDisposable pipelineDisposable;

        protected readonly TelemetryClient telemetry;

        protected ThrottledOperation(TimeSpan dueTime)
        {
            this.telemetry = new TelemetryClient();

            this.pipeline = new Subject<T>();

            // limit the number of events to every 5 seconds
            var connectable = this.pipeline
                .Throttle(TimeSpan.FromSeconds(5))
                .SelectMany(value => Observable.FromAsync(async () =>
                    {
                        try
                        {
                            await this.ProcessInternal(value);
                        }
                        catch (Exception e)
                        {
                            this.telemetry.TrackException(e);
                        }
                    }
                ))
                .Replay();

            this.pipelineDisposable = connectable.Connect();
        }

        internal void Process(T performance)
        {
            this.pipeline.OnNext(performance);
        }

        protected abstract Task ProcessInternal(T value);

        public void Dispose()
        {
            if (this.pipeline != null)
            {
                this.pipeline.Dispose();
                this.pipeline = null;
            }

            if (this.pipelineDisposable != null)
            {
                this.pipelineDisposable.Dispose();
                this.pipelineDisposable = null;
            }
        }
    }
}

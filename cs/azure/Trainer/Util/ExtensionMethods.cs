// --------------------------------------------------------------------------------------------------------------------
// <copyright file="ExtensionMethods.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using Microsoft.ApplicationInsights;
using System;
using System.Collections.Generic;
using System.Reactive.Linq;
using System.Threading.Tasks;

namespace VW.Azure.Trainer
{
    internal static class ExtensionMethods
    {
        internal static Task Trace(this Task task, TelemetryClient telemetry, string message)
        {
            return task.ContinueWith(t =>
            {
                telemetry.TrackTrace($"{message} completed: {t.Status}");
                if (t.IsFaulted)
                    telemetry.TrackException(t.Exception);
            });
        }

        internal static IObservable<IList<T>> Buffer<T>(this IObservable<T> source, int maxSize, Func<T, int> measure)
        {
            return Observable.Create<IList<T>>(obs =>
            {
                var state = new List<T>();
                var size = 0;
                return source.Subscribe(
                    onNext: v =>
                    {
                        size += measure(v);
                        state.Add(v);

                        if (size >= maxSize)
                        {
                            obs.OnNext(state);

                            state = new List<T>();
                            size = 0;
                        }
                    },
                    onError: e => obs.OnError(e),
                    onCompleted: () =>
                    {
                        if (state.Count > 0)
                        {
                            obs.OnNext(state);
                        }
                        obs.OnCompleted();
                    });
            });
        }
    }
}

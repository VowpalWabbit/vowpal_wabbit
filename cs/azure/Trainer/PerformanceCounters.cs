// --------------------------------------------------------------------------------------------------------------------
// <copyright file="PerformanceCounters.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using Microsoft.ApplicationInsights.Extensibility;
using Microsoft.ApplicationInsights.Extensibility.PerfCounterCollector;
using System;
using System.Linq;
using System.Diagnostics;
using Microsoft.ApplicationInsights;
using System.Collections.Generic;
using System.Text.RegularExpressions;
using System.Text;

namespace VW.Azure.Trainer
{
    /// <summary>
    /// Performance counters reporting various metrics.
    /// </summary>
    public sealed class PerformanceCounters : IDisposable
    {
        /// <summary>
        /// Performance counter attribute to autmatically allow counter creation.
        /// </summary>
        public class PerformanceCounterTypeAttribute : Attribute
        {
            /// <summary>
            /// Initializes a new <see cref="PerformanceCounterTypeAttribute"/> instance.
            /// </summary>
            public PerformanceCounterTypeAttribute(PerformanceCounterType type, string name = null)
            {
                this.Type = type;
                this.Name = name;
            }

            /// <summary>
            /// The performance counter type.
            /// </summary>
            public PerformanceCounterType Type { get; private set; }

            /// <summary>
            /// The desired name for the performance counter.
            /// </summary>
            public string Name { get; private set; }
        }

        private const string category = "Online Trainer";

        static PerformanceCounters()
        {
            try
            {
                if (PerformanceCounterCategory.Exists(category))
                    PerformanceCounterCategory.Delete(category);

                // order to be sure that *Base follows counter
                var props = typeof(PerformanceCounters)
                    .GetProperties()
                    .Where(p => p.PropertyType == typeof(PerformanceCounter))
                    .OrderBy(p => p.Name).ToList();

                var counterCollection = new CounterCreationDataCollection();

                foreach (var p in props)
                {
                    var attr = (PerformanceCounterTypeAttribute)p.GetCustomAttributes(typeof(PerformanceCounterTypeAttribute), true).First();
                    counterCollection.Add(new CounterCreationData() { CounterName = p.Name, CounterHelp = string.Empty, CounterType = attr.Type });
                }

                PerformanceCounterCategory.Create(category, "Online Trainer Perf Counters", PerformanceCounterCategoryType.MultiInstance, counterCollection);
            }
            catch (Exception e)
            {
                new TelemetryClient().TrackException(e);
            }
        }

        /// <summary>
        /// Initializes a new <see cref="PerformanceCounters"/> instance.
        /// </summary>
        public PerformanceCounters(string instance)
        {
            try
            {
                var perfCollectorModule = new PerformanceCollectorModule();
                var props = typeof(PerformanceCounters)
                    .GetProperties()
                    .Where(p => p.PropertyType == typeof(PerformanceCounter));

                var all = new List<PerformanceCounter>();
                foreach (var p in props)
                {
                    var counter = new PerformanceCounter(category, p.Name, instance, false);
                    p.SetValue(this, counter);
                    counter.RawValue = 0;
                    all.Add(counter);

                    if (!p.Name.EndsWith("Base", StringComparison.Ordinal))
                    {
                        var perfCounterSpec = $"\\{category}({instance})\\{p.Name}";
                        var reportAs = p.Name
                            .Replace('_', ' ')
                            .Replace("Per", "/");

                        // http://i1.blogs.msdn.com/b/visualstudioalm/archive/2015/04/01/application-insights-choose-your-own-performance-counters.aspx
                        // Currently, metric names may only contain letters, round brackets, forward slashes, hyphens, underscores, spaces and dots.
                        var reportAsStringBuilder = new StringBuilder(reportAs);
                        foreach (Match match in Regex.Matches(reportAs, "[0-9]"))
                            reportAsStringBuilder[match.Index] = (char)('A' + (match.Groups[0].Value[0] - '0'));

                        perfCollectorModule.Counters.Add(new PerformanceCounterCollectionRequest(perfCounterSpec, reportAsStringBuilder.ToString()));
                    }
                }

                perfCollectorModule.Initialize(TelemetryConfiguration.Active);

                this.All = all.ToArray();
            }
            catch (Exception e)
            {
                new TelemetryClient().TrackException(e);
            }
        }

        /// <summary>
        /// Disposes performance counter native resources.
        /// </summary>
        public void Dispose()
        {
            var props = typeof(PerformanceCounters)
                .GetProperties()
                .Where(p => p.PropertyType == typeof(IDisposable));

            foreach (var p in props)
            {
                var perfCounter = (IDisposable)p.GetValue(this);
                
                if (perfCounter != null)
                {
                    perfCounter.Dispose();
                    p.SetValue(this, null);
                }
            }
        }

        /// <summary>
        /// List of all online trainer performance counters.
        /// </summary>
        public PerformanceCounter[] All { get; private set; }

        /// <summary>
        /// Number of active Azure EventHub event processors.
        /// </summary>
        [PerformanceCounterType(PerformanceCounterType.NumberOfItems32)]
        public PerformanceCounter EventHub_Processors { get; private set; }

        /// <summary>
        /// Number of cached features.
        /// </summary>
        [PerformanceCounterType(PerformanceCounterType.NumberOfItems32)]
        public PerformanceCounter Features_Cached { get; private set; }

        /// <summary>
        /// Number of pending feature requests. Features referenced by ID, which have not yet occured in the stream.
        /// </summary>
        [PerformanceCounterType(PerformanceCounterType.NumberOfItems64)]
        public PerformanceCounter Feature_Requests_Pending { get; private set; }

        /// <summary>
        /// Number of feature requests discarded (e.g. due to timeout hit).
        /// </summary>
        [PerformanceCounterType(PerformanceCounterType.NumberOfItems64)]
        public PerformanceCounter Feature_Requests_Discarded { get; private set; }

        /// <summary>
        /// Total number of batches received by stage 0.
        /// </summary>
        [PerformanceCounterType(PerformanceCounterType.NumberOfItems64)]
        public PerformanceCounter Stage0_Batches_Total { get; private set; }

        /// <summary>
        /// Number of batches received per second by stage 0.
        /// </summary>
        [PerformanceCounterType(PerformanceCounterType.RateOfCountsPerSecond64)]
        public PerformanceCounter Stage0_BatchesPerSec { get; private set; }

        /// <summary>
        /// Average size of batches received per second by stage 0.
        /// </summary>
        [PerformanceCounterType(PerformanceCounterType.AverageCount64)]
        public PerformanceCounter Stage0_Batches_Size { get; private set; }

        /// <summary>
        /// Average (base) size of batches received per second by stage 0.
        /// </summary>
        [PerformanceCounterType(PerformanceCounterType.AverageBase)]
        public PerformanceCounter Stage0_Batches_SizeBase { get; private set; }

        /// <summary>
        /// Bytes/sec received by stage 0.
        /// </summary>
        [PerformanceCounterType(PerformanceCounterType.RateOfCountsPerSecond64)]
        public PerformanceCounter Stage0_IncomingBytesPerSec { get; private set; }

        /// <summary>
        /// Number of JSON lines in stage 1.
        /// </summary>
        [PerformanceCounterType(PerformanceCounterType.NumberOfItems64)]
        public PerformanceCounter Stage1_JSON_Queue { get; private set; }

        /// <summary>
        /// Number of JSON lines deserialized per second in stage 1.
        /// </summary>
        [PerformanceCounterType(PerformanceCounterType.RateOfCountsPerSecond64)]
        public PerformanceCounter Stage1_JSON_DeserializePerSec { get; private set; }

        /// <summary>
        /// Number of examples queued up for learning.
        /// </summary>
        [PerformanceCounterType(PerformanceCounterType.NumberOfItems64)]
        public PerformanceCounter Stage2_Learn_Queue { get; private set; }

        /// <summary>
        /// Total number of examples learned so far.
        /// </summary>
        [PerformanceCounterType(PerformanceCounterType.NumberOfItems64)]
        public PerformanceCounter Stage2_Learn_Total { get; private set; }

        /// <summary>
        /// Number of examples learned per second.
        /// </summary>
        [PerformanceCounterType(PerformanceCounterType.RateOfCountsPerSecond64)]
        public PerformanceCounter Stage2_Learn_ExamplesPerSec { get; private set; }

        /// <summary>
        /// Number of features learned per second.
        /// </summary>
        [PerformanceCounterType(PerformanceCounterType.RateOfCountsPerSecond64)]
        public PerformanceCounter Stage2_Learn_FeaturesPerSec { get; private set; }

        /// <summary>
        /// Total number of faulty examples encountered so far.
        /// </summary>
        [PerformanceCounterType(PerformanceCounterType.NumberOfItems64)]
        public PerformanceCounter Stage2_Faulty_Examples_Total { get; private set; }

        /// <summary>
        /// Number of faulty examples encountered per second.
        /// </summary>
        [PerformanceCounterType(PerformanceCounterType.RateOfCountsPerSecond64)]
        public PerformanceCounter Stage2_Faulty_ExamplesPerSec { get; private set; }

        /// <summary>
        /// Number of checkpoint requests queued up.
        /// </summary>
        [PerformanceCounterType(PerformanceCounterType.NumberOfItems64)]
        public PerformanceCounter Stage3_Checkpoint_Queue { get; private set; }

        /// <summary>
        /// Total number of evaluation outputs produced.
        /// </summary>
        [PerformanceCounterType(PerformanceCounterType.NumberOfItems64)]
        public PerformanceCounter Stage4_Evaluation_Total { get; private set; }

        /// <summary>
        /// Number of evaluation produced per second.
        /// </summary>
        [PerformanceCounterType(PerformanceCounterType.RateOfCountsPerSecond64)]
        public PerformanceCounter Stage4_Evaluation_PerSec { get; private set; }

        /// <summary>
        /// Total number of evaluation batches produces so far.
        /// </summary>
        [PerformanceCounterType(PerformanceCounterType.NumberOfItems64)]
        public PerformanceCounter Stage4_Evaluation_BatchesTotal { get; private set; }

        /// <summary>
        /// Number of evaluation batches produced per second.
        /// </summary>
        [PerformanceCounterType(PerformanceCounterType.RateOfCountsPerSecond64)]
        public PerformanceCounter Stage4_Evaluation_BatchesPerSec { get; private set; }

        /// <summary>
        /// Average example latency.
        /// </summary>
        [PerformanceCounterType(PerformanceCounterType.AverageTimer32)]
        public PerformanceCounter AverageExampleLatency { get; private set; }

        /// <summary>
        /// Average (base) example latency.
        /// </summary>
        [PerformanceCounterType(PerformanceCounterType.AverageBase)]
        public PerformanceCounter AverageExampleLatencyBase { get; private set; }
    }
}

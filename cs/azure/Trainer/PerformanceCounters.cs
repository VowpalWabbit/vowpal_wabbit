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
        public class PerformanceCounterTypeAttribute : Attribute
        {
            public PerformanceCounterTypeAttribute(PerformanceCounterType type, string name = null)
            {
                this.Type = type;
                this.Name = name;
            }

            public PerformanceCounterType Type { get; private set; }

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

        public PerformanceCounter[] All { get; private set; }

        [PerformanceCounterType(PerformanceCounterType.NumberOfItems32)]
        public PerformanceCounter EventHub_Processors { get; private set; }


        [PerformanceCounterType(PerformanceCounterType.NumberOfItems32)]
        public PerformanceCounter Features_Cached { get; private set; }

        [PerformanceCounterType(PerformanceCounterType.NumberOfItems64)]
        public PerformanceCounter Feature_Requests_Pending { get; private set; }

        [PerformanceCounterType(PerformanceCounterType.NumberOfItems64)]
        public PerformanceCounter Feature_Requests_Discarded { get; private set; }


        [PerformanceCounterType(PerformanceCounterType.NumberOfItems64)]
        public PerformanceCounter Stage0_Batches_Total { get; private set; }

        [PerformanceCounterType(PerformanceCounterType.RateOfCountsPerSecond64)]
        public PerformanceCounter Stage0_BatchesPerSec { get; private set; }

        [PerformanceCounterType(PerformanceCounterType.AverageCount64)]
        public PerformanceCounter Stage0_Batches_Size { get; private set; }

        [PerformanceCounterType(PerformanceCounterType.AverageBase)]
        public PerformanceCounter Stage0_Batches_SizeBase { get; private set; }

        [PerformanceCounterType(PerformanceCounterType.RateOfCountsPerSecond64)]
        public PerformanceCounter Stage0_IncomingBytesPerSec { get; private set; }


        [PerformanceCounterType(PerformanceCounterType.NumberOfItems64)]
        public PerformanceCounter Stage1_JSON_Queue { get; private set; }

        [PerformanceCounterType(PerformanceCounterType.RateOfCountsPerSecond64)]
        public PerformanceCounter Stage1_JSON_DeserializePerSec { get; private set; }


        [PerformanceCounterType(PerformanceCounterType.NumberOfItems64)]
        public PerformanceCounter Stage2_Learn_Queue { get; private set; }

        [PerformanceCounterType(PerformanceCounterType.NumberOfItems64)]
        public PerformanceCounter Stage2_Learn_Total { get; private set; }

        [PerformanceCounterType(PerformanceCounterType.RateOfCountsPerSecond64)]
        public PerformanceCounter Stage2_Learn_ExamplesPerSec { get; private set; }

        [PerformanceCounterType(PerformanceCounterType.RateOfCountsPerSecond64)]
        public PerformanceCounter Stage2_Learn_FeaturesPerSec { get; private set; }

        [PerformanceCounterType(PerformanceCounterType.NumberOfItems64)]
        public PerformanceCounter Stage2_Faulty_Examples_Total { get; private set; }

        [PerformanceCounterType(PerformanceCounterType.RateOfCountsPerSecond64)]
        public PerformanceCounter Stage2_Faulty_ExamplesPerSec { get; private set; }


        [PerformanceCounterType(PerformanceCounterType.NumberOfItems64)]
        public PerformanceCounter Stage3_Checkpoint_Queue { get; private set; }


        [PerformanceCounterType(PerformanceCounterType.NumberOfItems64)]
        public PerformanceCounter Stage4_Evaluation_Total { get; private set; }

        [PerformanceCounterType(PerformanceCounterType.RateOfCountsPerSecond64)]
        public PerformanceCounter Stage4_Evaluation_PerSec { get; private set; }

        [PerformanceCounterType(PerformanceCounterType.NumberOfItems64)]
        public PerformanceCounter Stage4_Evaluation_BatchesTotal { get; private set; }

        [PerformanceCounterType(PerformanceCounterType.RateOfCountsPerSecond64)]
        public PerformanceCounter Stage4_Evaluation_BatchesPerSec { get; private set; }


        [PerformanceCounterType(PerformanceCounterType.AverageTimer32)]
        public PerformanceCounter AverageExampleLatency { get; private set; }

        [PerformanceCounterType(PerformanceCounterType.AverageBase)]
        public PerformanceCounter AverageExampleLatencyBase { get; private set; }
    }
}

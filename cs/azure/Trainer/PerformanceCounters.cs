using Microsoft.ApplicationInsights.Extensibility;
using Microsoft.ApplicationInsights.Extensibility.PerfCounterCollector;
using System;
using System.Linq;
using System.Diagnostics;
using Microsoft.ApplicationInsights;

namespace VowpalWabbit.Azure.Trainer
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
                var props = typeof(PerformanceCounters).GetProperties().OrderBy(p => p.Name).ToList();

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
                foreach (var p in typeof(PerformanceCounters).GetProperties())
                {
                    var counter = new PerformanceCounter(category, p.Name, instance, false);
                    p.SetValue(this, counter);
                    counter.RawValue = 0;

                    if (!p.Name.EndsWith("Base", StringComparison.Ordinal))
                    {
                        var perfCounterSpec = $"\\{category}({instance})\\{p.Name}";
                        var reportAs = p.Name
                            .Replace('_', ' ')
                            .Replace("Per", "/");

                        perfCollectorModule.Counters.Add(new PerformanceCounterCollectionRequest(perfCounterSpec, reportAs));
                    }
                }

                perfCollectorModule.Initialize(TelemetryConfiguration.Active);
            }
            catch (Exception e)
            {
                new TelemetryClient().TrackException(e);
            }
        }

        public void Dispose()
        {
            foreach (var p in typeof(PerformanceCounters).GetProperties())
            {
                var perfCounter = (IDisposable)p.GetValue(this);
                
                if (perfCounter != null)
                {
                    perfCounter.Dispose();
                    p.SetValue(this, null);
                }
            }
        }

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


        [PerformanceCounterType(PerformanceCounterType.AverageTimer32)]
        public PerformanceCounter AverageExampleLatency { get; private set; }

        [PerformanceCounterType(PerformanceCounterType.AverageBase)]
        public PerformanceCounter AverageExampleLatencyBase { get; private set; }
    }
}

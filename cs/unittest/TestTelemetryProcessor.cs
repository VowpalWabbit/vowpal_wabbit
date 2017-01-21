using Microsoft.ApplicationInsights.Channel;
using Microsoft.ApplicationInsights.DataContracts;
using Microsoft.ApplicationInsights.Extensibility;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace cs_unittest
{
    public class TestTelemetryProcessor: ITelemetryProcessor
    {
        private ITelemetryProcessor Next { get; set; }

        public SynchronizedCollection<ExceptionTelemetry> Exceptions { get; private set; }

        // Link processors to each other in a chain.
        public TestTelemetryProcessor(ITelemetryProcessor next, SynchronizedCollection<ExceptionTelemetry> exceptions)
        {
            this.Next = next;
            this.Exceptions = exceptions;
        }

        public void Process(ITelemetry item)
        {
            var et = item as ExceptionTelemetry;
            if (et != null)
            {
                this.Exceptions.Add(et);
                Console.WriteLine($"Exception: {et.Message}.  {et.Exception.StackTrace}");
            }

            this.Next.Process(item);
        }
    }
}

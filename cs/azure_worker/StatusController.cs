// --------------------------------------------------------------------------------------------------------------------
// <copyright file="StatusController.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using Newtonsoft.Json.Linq;
using System.Linq;
using System.Web.Http;
using VW.Azure.Trainer;

namespace VW.Azure.Worker
{
    /// <summary>
    /// HTTP Front end to expose performance statistics.
    /// </summary>
    public class StatusController : ApiController
    {
        private LearnEventProcessorHost trainProcessorHost;

        public StatusController(LearnEventProcessorHost trainProcessorHost)
        {
            this.trainProcessorHost = trainProcessorHost;
        }

        public IHttpActionResult Get()
        {
            var perfCounts = this.trainProcessorHost.PerformanceCounters;

            if (perfCounts == null)
                return Json(new { Message = "Not yet initialized." });

            var status = new JObject(perfCounts.All.Select(pc => new JProperty(pc.CounterName, pc.RawValue)));
            status.Add(new JProperty("LastStartDateTimeUtc", this.trainProcessorHost.LastStartDateTimeUtc));
            return Json(status);
        }
    }
}

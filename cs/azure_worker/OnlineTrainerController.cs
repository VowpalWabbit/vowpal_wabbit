// --------------------------------------------------------------------------------------------------------------------
// <copyright file="OnlineTrainerController.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using Microsoft.ApplicationInsights;
using Microsoft.Azure;
using System.Linq;
using System.Web.Http;
using VW.Azure.Trainer;

namespace VW.Azure.Worker
{
    public class OnlineTrainerController : ApiController
    {
        // injected through Unity
        protected readonly LearnEventProcessorHost trainProcessorHost;
        protected readonly string adminToken;
        protected readonly TelemetryClient telemetry;

        public OnlineTrainerController(LearnEventProcessorHost trainProcessorFactory)
        {
            this.telemetry = new TelemetryClient();
            this.trainProcessorHost = trainProcessorFactory;
            this.adminToken = CloudConfigurationManager.GetSetting("AdminToken");
        }

        protected bool TryAuthorize()
        {
            var header = this.Request.Headers.SingleOrDefault(x => x.Key == "Authorization");

            return !(header.Value == null || adminToken != header.Value.First());
        }
    }
}

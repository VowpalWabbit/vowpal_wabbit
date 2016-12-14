using Microsoft.ApplicationInsights;
using Microsoft.Azure;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;
using System.Web.Http;
using VowpalWabbit.Azure.Trainer;

namespace VowpalWabbit.Azure.Worker
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

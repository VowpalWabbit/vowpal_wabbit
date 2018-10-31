using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Rl.Net;

namespace Rl.Net.Cli
{
    internal class ReplayStepProvider : IDriverStepProvider<string>
    {
        private class ReplayStep : IStepContext<string>
        {
            [JsonProperty("EventId")]
            public string EventId
            {
                get;
                set;
            }

            [JsonProperty("c")]
            public JObject Context
            {
                get;
                set;
            }

            [JsonProperty("o")]
            public JObject[] Observations
            {
                get;
                set;
            }

            [JsonIgnore]
            public string DecisionContext => this.Context.ToString(Formatting.None);

            public string GetOutcome(long actionIndex, IEnumerable<ActionProbability> actionDistribution)
            {
                JToken observationValue = this.Observations?.First()?.SelectToken("v");

                return observationValue?.ToString(Formatting.None);
            }
        }

        public ReplayStepProvider(IEnumerable<string> dsJsonHistory)
        {
            this.DSJsonHistory = dsJsonHistory;
        }

        public IEnumerable<string> DSJsonHistory
        {
            get;
            private set;
        }

        internal static IStepContext<string> DeserializeReplayStep(string dsJson)
        {
            return JsonConvert.DeserializeObject<ReplayStep>(dsJson);
        }

        public IEnumerator<IStepContext<string>> GetEnumerator()
        {
            return this.DSJsonHistory.Select(DeserializeReplayStep).GetEnumerator();
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return this.GetEnumerator();
        }
    }
}

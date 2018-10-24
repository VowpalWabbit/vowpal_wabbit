using Microsoft.VisualStudio.TestTools.UnitTesting;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Rl.Net;

namespace Rl.Net.Cli.Test
{
    [TestClass]
    public class ReplayStepProviderTest
    {
        private static string PrepareJson(string raw)
        {
            return JToken.Parse(raw).ToString(Formatting.None);
        }

        [TestMethod]
        public void Test_DeserializeReplayStep_WithObservation()
        {
            // Arrange
            string expectedEventId = "my_experiment-t1-n10-f2-a4-0-0";
            string expectedContext = PrepareJson("{ \"GUser\":{\"f_int\":0,\"f_float\":0.5,\"f_str_0\":\"value_0\",\"f_str_1\":\"value_1\"}, \"_multi\": [ { \"TAction\":{\"a_f_0\":\"value_0\",\"a_f_1\":\"value_1\"}},{ \"TAction\":{\"a_f_0\":\"value_1\",\"a_f_1\":\"value_2\"}},{ \"TAction\":{\"a_f_0\":\"value_2\",\"a_f_1\":\"value_3\"}},{ \"TAction\":{\"a_f_0\":\"value_3\",\"a_f_1\":\"value_4\"}}] }");
            string expectedObservation = PrepareJson("0.000000");

            const string dsJsonWithObservation = "{\"_label_cost\":-0,\"_label_probability\":0.25,\"_label_Action\":1,\"_labelIndex\":0,\"o\":[{\"EventId\":\"my_experiment-t1-n10-f2-a4-0-0\",\"v\":0.000000}],\"Version\":\"1\",\"EventId\":\"my_experiment-t1-n10-f2-a4-0-0\",\"a\":[1,2,3,4],\"c\":{ \"GUser\":{\"f_int\":0,\"f_float\":0.5,\"f_str_0\":\"value_0\",\"f_str_1\":\"value_1\"}, \"_multi\": [ { \"TAction\":{\"a_f_0\":\"value_0\",\"a_f_1\":\"value_1\"}},{ \"TAction\":{\"a_f_0\":\"value_1\",\"a_f_1\":\"value_2\"}},{ \"TAction\":{\"a_f_0\":\"value_2\",\"a_f_1\":\"value_3\"}},{ \"TAction\":{\"a_f_0\":\"value_3\",\"a_f_1\":\"value_4\"}}] },\"p\":[0.250000,0.250000,0.250000,0.250000],\"VWState\":{\"m\":\"N/A\"}}";

            // Act
            IStepContext<string> stepContext = ReplayStepProvider.DeserializeReplayStep(dsJsonWithObservation);
            string actualObservation = stepContext.GetOutcome(0, new ActionProbability[0]);

            Assert.AreEqual(expectedEventId, stepContext.EventId, $"{nameof(stepContext.EventId)} is not properly deserialized.");
            Assert.AreEqual(expectedContext, stepContext.DecisionContext, $"{nameof(stepContext.DecisionContext)} is not properly deserialized.");
            Assert.AreEqual(expectedObservation, actualObservation, $"Observation is not properly deserialized.");
        }

        [TestMethod]
        public void Test_DeserializeReplayStep_WithoutObservation()
        {
            // Arrange
            string expectedEventId = "my_experiment-t1-n10-f2-a4-0-0";
            string expectedContext = PrepareJson("{ \"GUser\":{\"f_int\":0,\"f_float\":0.5,\"f_str_0\":\"value_0\",\"f_str_1\":\"value_1\"}, \"_multi\": [ { \"TAction\":{\"a_f_0\":\"value_0\",\"a_f_1\":\"value_1\"}},{ \"TAction\":{\"a_f_0\":\"value_1\",\"a_f_1\":\"value_2\"}},{ \"TAction\":{\"a_f_0\":\"value_2\",\"a_f_1\":\"value_3\"}},{ \"TAction\":{\"a_f_0\":\"value_3\",\"a_f_1\":\"value_4\"}}] }");
            string expectedObservation = null;

            const string dsJsonWithObservation = "{\"_label_cost\":-0,\"_label_probability\":0.25,\"_label_Action\":1,\"_labelIndex\":0,\"Version\":\"1\",\"EventId\":\"my_experiment-t1-n10-f2-a4-0-0\",\"a\":[1,2,3,4],\"c\":{ \"GUser\":{\"f_int\":0,\"f_float\":0.5,\"f_str_0\":\"value_0\",\"f_str_1\":\"value_1\"}, \"_multi\": [ { \"TAction\":{\"a_f_0\":\"value_0\",\"a_f_1\":\"value_1\"}},{ \"TAction\":{\"a_f_0\":\"value_1\",\"a_f_1\":\"value_2\"}},{ \"TAction\":{\"a_f_0\":\"value_2\",\"a_f_1\":\"value_3\"}},{ \"TAction\":{\"a_f_0\":\"value_3\",\"a_f_1\":\"value_4\"}}] },\"p\":[0.250000,0.250000,0.250000,0.250000],\"VWState\":{\"m\":\"N/A\"}}";

            // Act
            IStepContext<string> stepContext = ReplayStepProvider.DeserializeReplayStep(dsJsonWithObservation);
            string actualObservation = stepContext.GetOutcome(0, new ActionProbability[0]);

            Assert.AreEqual(expectedEventId, stepContext.EventId, $"{nameof(stepContext.EventId)} is not properly deserialized.");
            Assert.AreEqual(expectedContext, stepContext.DecisionContext, $"{nameof(stepContext.DecisionContext)} is not properly deserialized.");
            Assert.AreEqual(expectedObservation, actualObservation, $"Observation is not properly deserialized.");
        }
    }
}

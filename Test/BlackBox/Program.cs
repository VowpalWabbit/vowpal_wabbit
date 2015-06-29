using MultiWorldTesting;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using TestCommon;

namespace BlackBoxTests
{
    class Program
    {
        static void Main(string[] args)
        {
            if (args.Length != 1)
            {
                return;
            }

            var configList = (JContainer)JsonConvert.DeserializeObject(File.ReadAllText(args[0]));
            for (int i = 0; i < configList.Count; i++)
            {
                var config = (JObject)configList[i];

                switch (config["Type"].Value<int>())
                {
                    case 0:
                        TestPrg(config);
                        break;
                    case 1:
                        TestHash(config);
                        break;
                    case 2:
                        TestEpsilonGreedy(config);
                        break;
                }
            }
        }

        static void TestPrg(JObject config)
        {
            var seed = config["Seed"].Value<ulong>();
            var iterations = config["Iterations"].Value<int>();
            var uniformInterval = JsonConvert.DeserializeObject<Tuple<uint, uint>>(config["UniformInterval"].ToString());
            var outputFile = config["OutputFile"].Value<string>();

            var outputLines = new List<string>();
            var prg = new PRG(seed);

            for (int i = 0; i < iterations; i++)
            {
                if (uniformInterval != null)
                {
                    uint random = prg.UniformInt(uniformInterval.Item1, uniformInterval.Item2);
                    outputLines.Add(random.ToString());
                }
                else
                {
                    float random = prg.UniformUnitInterval();
                    outputLines.Add(random.ToString());
                }
            }

            File.AppendAllLines(outputFile, outputLines);
        }

        static void TestHash(JObject config)
        {
            var values = config["Values"].ToObject<string[]>();
            var outputFile = config["OutputFile"].Value<string>();

            File.AppendAllLines(outputFile, values.Select(v => MurMurHash3.ComputeIdHash(v).ToString()));
        }

        static void TestEpsilonGreedy(JObject config)
        {
            var outputFile = config["OutputFile"].Value<string>();
            var appId = config["AppId"].Value<string>();
            var numActions = config["NumberOfActions"].Value<uint>();
            var experimentalUnitIdList = config["ExperimentalUnitIdList"].ToObject<string[]>();
            var epsilon = config["Epsilon"].Value<float>();
            JToken configPolicy = config["PolicyConfiguration"];
            var policyType = configPolicy["PolicyType"].Value<int>();

            switch (config["ContextType"].Value<int>())
            {
                case 0: // fixed action context
                {
                    var contextList = Enumerable
                        .Range(0, experimentalUnitIdList.Length)
                        .Select(i => new RegularTestContext { Id = i })
                        .ToArray();

                    ExploreEpsilonGreedy<RegularTestContext>(appId, policyType, configPolicy, epsilon, 
                        numActions, experimentalUnitIdList, contextList, outputFile);
                    
                    break;
                }
                case 1: // variable action context
                {
                    var contextList = Enumerable
                        .Range(0, experimentalUnitIdList.Length)
                        .Select(i => new VariableActionTestContext(numActions) { Id = i })
                        .ToArray();

                    ExploreEpsilonGreedy<VariableActionTestContext>(appId, policyType, configPolicy, epsilon,
                        numActions, experimentalUnitIdList, contextList, outputFile);

                    break;
                }
            }
        }

        static void ExploreEpsilonGreedy<TContext>
        (
            string appId,
            int policyType,
            JToken configPolicy,
            float epsilon,
            uint numActions,
            string[] experimentalUnitIdList,
            TContext[] contextList,
            string outputFile
        )
            where TContext : IStringContext
        {
            var recorder = new StringRecorder<TContext>();
            var mwt = new MwtExplorer<TContext>(appId, recorder);

            bool isVariableActionContext = typeof(IVariableActionContext).IsAssignableFrom(typeof(TContext));

            switch (policyType)
            {
                case 0: // fixed policy
                {
                    var policyAction = configPolicy["Action"].Value<uint>();

                    var policy = new TestPolicy<TContext> { ActionToChoose = policyAction };

                    var explorer = isVariableActionContext ?
                        new EpsilonGreedyExplorer<TContext>(policy, epsilon) :
                        new EpsilonGreedyExplorer<TContext>(policy, epsilon, numActions);

                    for (int i = 0; i < experimentalUnitIdList.Length; i++)
                    {
                        mwt.ChooseAction(explorer, experimentalUnitIdList[i], contextList[i]);
                    }

                    File.AppendAllText(outputFile, recorder.GetRecording());

                    break;
                }
            }
        }
    }
}

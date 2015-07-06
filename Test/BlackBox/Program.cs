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
                    case 3:
                        TestTauFirst(config);
                        break;
                    case 4:
                        TestSoftmax(config);
                        break;
                    case 5:
                        TestGeneric(config);
                        break;
                    case 6:
                        TestBootstrap(config);
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

        // TODO: refactor
        static void TestTauFirst(JObject config)
        {
            var outputFile = config["OutputFile"].Value<string>();
            var appId = config["AppId"].Value<string>();
            var numActions = config["NumberOfActions"].Value<uint>();
            var experimentalUnitIdList = config["ExperimentalUnitIdList"].ToObject<string[]>();
            var tau = config["Tau"].Value<uint>();
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

                        ExploreTauFirst<RegularTestContext>(appId, policyType, configPolicy, tau,
                            numActions, experimentalUnitIdList, contextList, outputFile);

                        break;
                    }
                case 1: // variable action context
                    {
                        var contextList = Enumerable
                            .Range(0, experimentalUnitIdList.Length)
                            .Select(i => new VariableActionTestContext(numActions) { Id = i })
                            .ToArray();

                        ExploreTauFirst<VariableActionTestContext>(appId, policyType, configPolicy, tau,
                            numActions, experimentalUnitIdList, contextList, outputFile);

                        break;
                    }
            }
        }

        // TODO: refactor
        static void TestSoftmax(JObject config)
        {
            var outputFile = config["OutputFile"].Value<string>();
            var appId = config["AppId"].Value<string>();
            var numActions = config["NumberOfActions"].Value<uint>();
            var experimentalUnitIdList = config["ExperimentalUnitIdList"].ToObject<string[]>();
            var lambda = config["Lambda"].Value<float>();
            JToken configScorer = config["ScorerConfiguration"];
            var scorerType = configScorer["ScorerType"].Value<int>();

            switch (config["ContextType"].Value<int>())
            {
                case 0: // fixed action context
                {
                    var contextList = Enumerable
                        .Range(0, experimentalUnitIdList.Length)
                        .Select(i => new RegularTestContext { Id = i })
                        .ToArray();

                    ExploreSoftmax<RegularTestContext>(appId, scorerType, configScorer, lambda,
                        numActions, experimentalUnitIdList, contextList, outputFile);

                    break;
                }
                case 1: // variable action context
                {
                    var contextList = Enumerable
                        .Range(0, experimentalUnitIdList.Length)
                        .Select(i => new VariableActionTestContext(numActions) { Id = i })
                        .ToArray();

                    ExploreSoftmax<VariableActionTestContext>(appId, scorerType, configScorer, lambda,
                        numActions, experimentalUnitIdList, contextList, outputFile);

                    break;
                }
            }
        }

        // TODO: refactor
        static void TestGeneric(JObject config)
        {
            var outputFile = config["OutputFile"].Value<string>();
            var appId = config["AppId"].Value<string>();
            var numActions = config["NumberOfActions"].Value<uint>();
            var experimentalUnitIdList = config["ExperimentalUnitIdList"].ToObject<string[]>();
            JToken configScorer = config["ScorerConfiguration"];
            var scorerType = configScorer["ScorerType"].Value<int>();

            switch (config["ContextType"].Value<int>())
            {
                case 0: // fixed action context
                {
                    var contextList = Enumerable
                        .Range(0, experimentalUnitIdList.Length)
                        .Select(i => new RegularTestContext { Id = i })
                        .ToArray();

                    ExploreGeneric<RegularTestContext>(appId, scorerType, configScorer,
                        numActions, experimentalUnitIdList, contextList, outputFile);

                    break;
                }
                case 1: // variable action context
                {
                    var contextList = Enumerable
                        .Range(0, experimentalUnitIdList.Length)
                        .Select(i => new VariableActionTestContext(numActions) { Id = i })
                        .ToArray();

                    ExploreGeneric<VariableActionTestContext>(appId, scorerType, configScorer,
                        numActions, experimentalUnitIdList, contextList, outputFile);

                    break;
                }
            }
        }

        // TODO: refactor
        static void TestBootstrap(JObject config)
        {
            var outputFile = config["OutputFile"].Value<string>();
            var appId = config["AppId"].Value<string>();
            var numActions = config["NumberOfActions"].Value<uint>();
            var experimentalUnitIdList = config["ExperimentalUnitIdList"].ToObject<string[]>();
            var configPolicies = (JArray)config["PolicyConfigurations"];

            switch (config["ContextType"].Value<int>())
            {
                case 0: // fixed action context
                {
                    var contextList = Enumerable
                        .Range(0, experimentalUnitIdList.Length)
                        .Select(i => new RegularTestContext { Id = i })
                        .ToArray();

                    ExploreBootstrap<RegularTestContext>(appId, configPolicies,
                        numActions, experimentalUnitIdList, contextList, outputFile);

                    break;
                }
                case 1: // variable action context
                {
                    var contextList = Enumerable
                        .Range(0, experimentalUnitIdList.Length)
                        .Select(i => new VariableActionTestContext(numActions) { Id = i })
                        .ToArray();

                    ExploreBootstrap<VariableActionTestContext>(appId, configPolicies,
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

        static void ExploreTauFirst<TContext>
        (
            string appId,
            int policyType,
            JToken configPolicy,
            uint tau,
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
                        new TauFirstExplorer<TContext>(policy, tau) :
                        new TauFirstExplorer<TContext>(policy, tau, numActions);

                    for (int i = 0; i < experimentalUnitIdList.Length; i++)
                    {
                        mwt.ChooseAction(explorer, experimentalUnitIdList[i], contextList[i]);
                    }

                    File.AppendAllText(outputFile, recorder.GetRecording());

                    break;
                }
            }
        }

        static void ExploreSoftmax<TContext>
        (
            string appId,
            int policyType,
            JToken configPolicy,
            float lambda,
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
                case 0: // fixed all-equal scorer
                {
                    var scorerScore = configPolicy["Score"].Value<int>();

                    var scorer = new TestScorer<TContext>(scorerScore, numActions);

                    var explorer = isVariableActionContext ?
                        new SoftmaxExplorer<TContext>(scorer, lambda) :
                        new SoftmaxExplorer<TContext>(scorer, lambda, numActions);

                    for (int i = 0; i < experimentalUnitIdList.Length; i++)
                    {
                        mwt.ChooseAction(explorer, experimentalUnitIdList[i], contextList[i]);
                    }

                    File.AppendAllText(outputFile, recorder.GetRecording());

                    break;
                }
                case 1: // integer-progression scorer
                {
                    var scorerStartScore = configPolicy["Start"].Value<int>();

                    var scorer = new TestScorer<TContext>(scorerStartScore, numActions, uniform: false);

                    var explorer = isVariableActionContext ?
                        new SoftmaxExplorer<TContext>(scorer, lambda) :
                        new SoftmaxExplorer<TContext>(scorer, lambda, numActions);

                    for (int i = 0; i < experimentalUnitIdList.Length; i++)
                    {
                        mwt.ChooseAction(explorer, experimentalUnitIdList[i], contextList[i]);
                    }

                    File.AppendAllText(outputFile, recorder.GetRecording());

                    break;
                }
            }
        }

        static void ExploreGeneric<TContext>
        (
            string appId,
            int policyType,
            JToken configPolicy,
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
                case 0: // fixed all-equal scorer
                {
                    var scorerScore = configPolicy["Score"].Value<int>();

                    var scorer = new TestScorer<TContext>(scorerScore, numActions);

                    var explorer = isVariableActionContext ?
                        new GenericExplorer<TContext>(scorer) :
                        new GenericExplorer<TContext>(scorer, numActions);

                    for (int i = 0; i < experimentalUnitIdList.Length; i++)
                    {
                        mwt.ChooseAction(explorer, experimentalUnitIdList[i], contextList[i]);
                    }

                    File.AppendAllText(outputFile, recorder.GetRecording());

                    break;
                }
                case 1: // integer-progression scorer
                {
                    var scorerStartScore = configPolicy["Start"].Value<int>();

                    var scorer = new TestScorer<TContext>(scorerStartScore, numActions, uniform: false);

                    var explorer = isVariableActionContext ?
                        new GenericExplorer<TContext>(scorer) :
                        new GenericExplorer<TContext>(scorer, numActions);

                    for (int i = 0; i < experimentalUnitIdList.Length; i++)
                    {
                        mwt.ChooseAction(explorer, experimentalUnitIdList[i], contextList[i]);
                    }

                    File.AppendAllText(outputFile, recorder.GetRecording());

                    break;
                }
            }
        }

        static void ExploreBootstrap<TContext>
        (
            string appId,
            JArray configPolicies,
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

            var policies = new List<IPolicy<TContext>>();

            for (int p = 0; p < configPolicies.Count; p++)
            {
                JToken configPolicy = configPolicies[p];
                switch (configPolicy["PolicyType"].Value<int>())
                {
                    case 0: // fixed policy
                    {
                        var policyAction = configPolicy["Action"].Value<uint>();
                        policies.Add(new TestPolicy<TContext> { ActionToChoose = policyAction });
                        break;
                    }
                }
            }

            var explorer = isVariableActionContext ?
                new BootstrapExplorer<TContext>(policies.ToArray()) :
                new BootstrapExplorer<TContext>(policies.ToArray(), numActions);

            for (int i = 0; i < experimentalUnitIdList.Length; i++)
            {
                mwt.ChooseAction(explorer, experimentalUnitIdList[i], contextList[i]);
            }

            File.AppendAllText(outputFile, recorder.GetRecording());
        }
    }
}

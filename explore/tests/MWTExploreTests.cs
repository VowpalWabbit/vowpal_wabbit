using System;
using System.Runtime.InteropServices;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using MultiWorldTesting;
using System.Collections.Generic;
using System.Linq;

namespace ExploreTests
{
    [TestClass]
    public class MWTExploreTests
    {
        /* 
        ** C# Tests do not need to be as extensive as those for C++. These tests should ensure
        ** the interactions between managed and native code are as expected.
        */
        [TestMethod]
        public void EpsilonGreedy()
        {
            uint numActions = 10;
            float epsilon = 0f;
            string uniqueKey = "ManagedTestId";

            TestRecorder recorder = new TestRecorder();
            TestPolicy policy = new TestPolicy();
            MwtExplorer<TestContext> mwtt = new MwtExplorer<TestContext>("mwt", recorder);
            TestContext testContext = new TestContext();
            testContext.Id = 100;

            var explorer = new EpsilonGreedyExplorer<TestContext>(policy, epsilon, numActions);

            uint expectedAction = policy.ChooseAction(testContext);

            uint chosenAction = mwtt.ChooseAction(explorer, uniqueKey, testContext);
            Assert.AreEqual(expectedAction, chosenAction);

            chosenAction = mwtt.ChooseAction(explorer, uniqueKey, testContext);
            Assert.AreEqual(expectedAction, chosenAction);

            var interactions = recorder.GetAllInteractions();
            Assert.AreEqual(2, interactions.Count);

            Assert.AreEqual(testContext.Id, interactions[0].Context.Id);
        }

        [TestMethod]
        public void TauFirst()
        {
            uint numActions = 10;
            uint tau = 0;
            string uniqueKey = "ManagedTestId";

            TestRecorder recorder = new TestRecorder();
            TestPolicy policy = new TestPolicy();
            MwtExplorer<TestContext> mwtt = new MwtExplorer<TestContext>("mwt", recorder);
            TestContext testContext = new TestContext();
            testContext.Id = 100;

            var explorer = new TauFirstExplorer<TestContext>(policy, tau, numActions);

            uint expectedAction = policy.ChooseAction(testContext);

            uint chosenAction = mwtt.ChooseAction(explorer, uniqueKey, testContext);
            Assert.AreEqual(expectedAction, chosenAction);

            var interactions = recorder.GetAllInteractions();
            Assert.AreEqual(0, interactions.Count);
        }

        [TestMethod]
        public void Bagging()
        {
            uint numActions = 10;
            uint numbags = 2;
            string uniqueKey = "ManagedTestId";

            TestRecorder recorder = new TestRecorder();
            TestPolicy[] policies = new TestPolicy[numbags];
            for (int i = 0; i < numbags; i++)
            {
                policies[i] = new TestPolicy(i * 2);
            }
            TestContext testContext1 = new TestContext() { Id = 99 };
            TestContext testContext2 = new TestContext() { Id = 100 };

            MwtExplorer<TestContext> mwtt = new MwtExplorer<TestContext>("mwt", recorder);
            var explorer = new BaggingExplorer<TestContext>(policies, numbags, numActions);

            uint expectedAction = policies[0].ChooseAction(testContext1);

            uint chosenAction = mwtt.ChooseAction(explorer, uniqueKey, testContext1);
            Assert.AreEqual(expectedAction, chosenAction);

            chosenAction = mwtt.ChooseAction(explorer, uniqueKey, testContext2);
            Assert.AreEqual(expectedAction, chosenAction);

            var interactions = recorder.GetAllInteractions();
            Assert.AreEqual(2, interactions.Count);

            Assert.AreEqual(testContext1.Id, interactions[0].Context.Id);
            Assert.AreEqual(testContext2.Id, interactions[1].Context.Id);
        }

        [TestMethod]
        public void Softmax()
        {
            uint numActions = 10;
            float lambda = 0.5f;
            TestRecorder recorder = new TestRecorder();
            TestScorer scorer = new TestScorer(numActions);

            MwtExplorer<TestContext> mwtt = new MwtExplorer<TestContext>("mwt", recorder);
            var explorer = new SoftmaxExplorer<TestContext>(scorer, lambda, numActions);

            uint numDecisions = (uint)(NumActions * Math.Log(NumActions * 1.0) + Math.Log(NumActionsCover * 1.0 / NumActions) * C * NumActions);
            uint[] actions = new uint[NumActions];

            Random rand = new Random();
            for (uint i = 0; i < numDecisions; i++)
            {
                uint chosenAction = mwtt.ChooseAction(explorer, rand.NextDouble().ToString(), new TestContext() { Id = (int)i });
                actions[ActionID.Make_ZeroBased(chosenAction)]++;
            }
            
            for (uint i = 0; i < NumActions; i++)
            {
                Assert.IsTrue(actions[i] > 0);
            }

            var interactions = recorder.GetAllInteractions();
            Assert.AreEqual(numDecisions, (uint)interactions.Count);

            for (int i = 0; i < numDecisions; i++)
            {
                Assert.AreEqual(i, interactions[i].Context.Id);
            }
        }

        [TestMethod]
        public void SoftmaxScores()
        {
            uint numActions = 10;
            float lambda = 0.5f;
            TestRecorder recorder = new TestRecorder();
            TestScorer scorer = new TestScorer(numActions, uniform: false);

            MwtExplorer<TestContext> mwtt = new MwtExplorer<TestContext>("mwt", recorder);
            var explorer = new SoftmaxExplorer<TestContext>(scorer, lambda, numActions);

            Random rand = new Random();
            mwtt.ChooseAction(explorer, rand.NextDouble().ToString(), new TestContext() { Id = 100 });
            mwtt.ChooseAction(explorer, rand.NextDouble().ToString(), new TestContext() { Id = 101 });
            mwtt.ChooseAction(explorer, rand.NextDouble().ToString(), new TestContext() { Id = 102 });

            var interactions = recorder.GetAllInteractions();
            
            Assert.AreEqual(3, interactions.Count);

            for (int i = 0; i < interactions.Count; i++)
            {
                // Scores are not equal therefore probabilities should not be uniform
                Assert.AreNotEqual(interactions[i].Probability, 1.0f / NumActions);
                Assert.AreEqual(100 + i, interactions[i].Context.Id);
            }
        }

        [TestMethod]
        public void Generic()
        {
            uint numActions = 10;
            string uniqueKey = "ManagedTestId";
            TestRecorder recorder = new TestRecorder();
            TestScorer scorer = new TestScorer(numActions);

            MwtExplorer<TestContext> mwtt = new MwtExplorer<TestContext>("mwt", recorder);
            var explorer = new GenericExplorer<TestContext>(scorer, numActions);

            TestContext testContext = new TestContext() { Id = 100 };
            uint chosenAction = mwtt.ChooseAction(explorer, uniqueKey, testContext);

            var interactions = recorder.GetAllInteractions();
            Assert.AreEqual(1, interactions.Count);
            Assert.AreEqual(testContext.Id, interactions[0].Context.Id);
        }

        [TestMethod]
        public void EndToEndEpsilonGreedy()
        {
            mwt.InitializeEpsilonGreedy(0.5f,
                new StatefulPolicyDelegate<int>(TestStatefulPolicyFunc), PolicyParams,
                NumActions);

            EndToEnd();
        }

        [TestMethod]
        public void EndToEndTauFirst()
        {
            mwt.InitializeTauFirst<int>(5,
                new StatefulPolicyDelegate<int>(TestStatefulPolicyFunc),
                PolicyParams,
                NumActions);

            EndToEnd();
        }

        [TestMethod]
        public void EndToEndBagging()
        {
            StatefulPolicyDelegate<int>[] funcs = new StatefulPolicyDelegate<int>[Bags];
            int[] funcParams = new int[Bags];
            for (int i = 0; i < Bags; i++)
            {
                funcs[i] = new StatefulPolicyDelegate<int>(TestStatefulPolicyFunc);
                funcParams[i] = PolicyParams;
            }

            mwt.InitializeBagging(Bags, funcs, funcParams, NumActions);

            EndToEnd();
        }

        [TestMethod]
        public void EndToEndSoftmax()
        {
            mwt.InitializeSoftmax<int>(0.5f,
                new StatefulScorerDelegate<int>(TestStatefulScorerFunc),
                PolicyParams, NumActions);

            EndToEnd();
        }

        [TestMethod]
        public void EndToEndGeneric()
        {
            mwt.InitializeGeneric<int>(
                new StatefulScorerDelegate<int>(TestStatefulScorerFunc),
                PolicyParams,
                NumActions);

            EndToEnd();
        }

        [TestMethod]
        public void CustomContextEpsilonStateful()
        {
            mwt.InitializeEpsilonGreedy<int>(0f,
               new StatefulPolicyDelegate<int>(TestStatefulPolicyFunc),
               PolicyParams,
               NumActions);

            ExploreWithCustomContext();
        }

        [TestMethod]
        public void CustomContextEpsilonStateless()
        {
            mwt.InitializeEpsilonGreedy(0f,
                new StatelessPolicyDelegate(TestStatelessPolicyFunc),
                NumActions);

            ExploreWithCustomContext();
        }

        [TestMethod]
        public void CustomContextTauFirstStateful()
        {
            mwt.InitializeTauFirst<int>(1,
                new StatefulPolicyDelegate<int>(TestStatefulPolicyFunc),
                PolicyParams,
                NumActions);

            ExploreWithCustomContext();
        }

        [TestMethod]
        public void CustomContextTauFirstStateless()
        {
            mwt.InitializeTauFirst(1,
                new StatelessPolicyDelegate(TestStatelessPolicyFunc),
                NumActions);

            ExploreWithCustomContext();
        }

        [TestMethod]
        public void CustomContextBaggingStateful()
        {
            uint bags = 2;
            StatefulPolicyDelegate<int>[] funcs = new StatefulPolicyDelegate<int>[bags];
            int[] funcParams = new int[bags];
            for (int i = 0; i < bags; i++)
            {
                funcs[i] = new StatefulPolicyDelegate<int>(TestStatefulPolicyFunc);
                funcParams[i] = PolicyParams;
            }

            mwt.InitializeBagging(bags, funcs, funcParams, NumActions);
            ExploreWithCustomContext();
        }

        [TestMethod]
        public void CustomContextBaggingStateless()
        {
            uint bags = 2;
            StatelessPolicyDelegate[] funcs = new StatelessPolicyDelegate[bags];
            for (int i = 0; i < bags; i++)
            {
                funcs[i] = new StatelessPolicyDelegate(TestStatelessPolicyFunc);
            }

            mwt.InitializeBagging(bags, funcs, NumActions);
            ExploreWithCustomContext();
        }

        [TestMethod]
        public void CustomContextSoftmaxStateful()
        {
            mwt.InitializeSoftmax<int>(0.5f,
                new StatefulScorerDelegate<int>(TestStatefulScorerFunc),
                PolicyParams, NumActions);

            ExploreWithCustomContext();

        }

        [TestMethod]
        public void CustomContextSoftmaxStateless()
        {
            mwt.InitializeSoftmax(0.5f,
                new StatelessScorerDelegate(TestStatelessScorerFunc),
                NumActions);

            ExploreWithCustomContext();
        }

        [TestMethod]
        public void CustomContextGenericStateful()
        {
            mwt.InitializeGeneric<int>(
                new StatefulScorerDelegate<int>(TestStatefulScorerFunc),
                PolicyParams,
                NumActions);

            ExploreWithCustomContext();
        }

        [TestMethod]
        public void CustomContextGenericStateless()
        {
            mwt.InitializeGeneric(
                new StatelessScorerDelegate(TestStatelessScorerFunc),
                NumActions);

            ExploreWithCustomContext();
        }

        [TestInitialize]
        public void TestInitialize()
        {
            mwt = new OldMwtExplorer("test");

            features = new Feature[2];
            features[0].Value = 0.5f;
            features[0].Id = 1;
            features[1].Value = 0.9f;
            features[1].Id = 2;

            context = new OldSimpleContext(features, "Other C# test context");
        }

        [TestCleanup]
        public void TestCleanup()
        {
            mwt.Uninitialize();
        }

        private void ExploreWithCustomContext()
        {
            OldTestContext testContext = new OldTestContext();
            testContext.SetFeatures();

            uint chosenAction = mwt.ChooseAction(UniqueKey, testContext);

            Interaction[] interactions = mwt.GetAllInteractions();
            Assert.AreEqual(1, interactions.Length);

            BaseContext returnedContext = interactions[0].GetContext();
            Assert.IsTrue(returnedContext is OldTestContext);
            Assert.AreEqual(testContext, returnedContext);

            Feature[] originalFeatures = testContext.GetFeatures();
            Feature[] returnedFeatures = returnedContext.GetFeatures();

            Assert.AreEqual(originalFeatures.Length, returnedFeatures.Length);
            Assert.AreEqual(3, returnedFeatures.Length);
            for (int i = 0; i < originalFeatures.Length; i++)
            {
                Assert.AreEqual(originalFeatures[i].Id, returnedFeatures[i].Id);
                Assert.AreEqual(originalFeatures[i].Value, returnedFeatures[i].Value);
            }
        }

        private void EndToEnd()
        {
            Random rand = new Random();

            List<float> rewards = new List<float>();
            for (int i = 0; i < 1000; i++)
            {
                Feature[] f = new Feature[rand.Next(800, 1201)];
                for (int j = 0; j < f.Length; j++)
                {
                    f[j].Id = (uint)(j + 1);
                    f[j].Value = (float)rand.NextDouble();
                }

                OldSimpleContext c = new OldSimpleContext(f, null);
                mwt.ChooseAction(i.ToString(), c);

                rewards.Add((float)rand.NextDouble());
            }

            Interaction[] partialInteractions = mwt.GetAllInteractions();

            MwtRewardReporter mrr = new MwtRewardReporter(partialInteractions);
            for (int i = 0; i < partialInteractions.Length; i++)
            {
                Assert.AreEqual(true, mrr.ReportReward(partialInteractions[i].GetId(), rewards[i]));
            }

            Interaction[] completeInteractions = mrr.GetAllInteractions();
            MwtOptimizer mop = new MwtOptimizer(completeInteractions, NumActions);

            string modelFile = "model";

            mop.OptimizePolicyVWCSOAA(modelFile);

            Assert.IsTrue(System.IO.File.Exists(modelFile));

            float evaluatedValue = mop.EvaluatePolicyVWCSOAA(modelFile);

            Assert.IsFalse(float.IsNaN(evaluatedValue));

            System.IO.File.Delete(modelFile);
        }

        private static UInt32 TestStatefulPolicyFunc(int policyParams, BaseContext context)
        {
            return ActionID.Make_OneBased((uint)(policyParams + context.GetFeatures().Length) % MWTExploreTests.NumActions);
        }

        private static UInt32 TestStatelessPolicyFunc(BaseContext context)
        {
            return ActionID.Make_OneBased((uint)context.GetFeatures().Length % MWTExploreTests.NumActions);
        }

        private static void TestStatefulScorerFunc(int policyParams, BaseContext applicationContext, float[] scores)
        {
            for (uint i = 0; i < scores.Length; i++)
            {
                scores[i] = policyParams;
            }
        }

        private static void TestStatelessScorerFunc(BaseContext applicationContext, float[] scores)
        {
            for (uint i = 0; i < scores.Length; i++)
            {
                scores[i] = applicationContext.GetFeatures().Length;
            }
        }

        private static void NonUniformStatefulScorerFunc(int policyParams, BaseContext applicationContext, float[] scores)
        {
            for (uint i = 0; i < scores.Length; i++)
            {
                scores[i] = policyParams + i;
            }
        }

        private static void NonUniformStatelessScorerFunc(BaseContext applicationContext, float[] scores)
        {
            for (uint i = 0; i < scores.Length; i++)
            {
                scores[i] = i;
            }
        }

        private static readonly uint NumActionsCover = 100;
        private static readonly float C = 5;

        private static readonly uint NumActions = 10;
        private static readonly float Epsilon = 0;
        private static readonly uint Tau = 0;
        private static readonly uint Bags = 2;
        private static readonly float Lambda = 0.5f;
        private static readonly int PolicyParams = 1003;
        private static readonly string UniqueKey = "ManagedTestId";

        private OldMwtExplorer mwt;
        private Feature[] features;
        private OldSimpleContext context;
    }

    public class OldTestContext : BaseContext
    {
        public OldTestContext()
        { 
            features = new Feature[] 
            { 
                new Feature() { Id = 9999, Value = 9999f },
            };
        }

        public void SetFeatures()
        {
            features = new Feature[] 
            { 
                new Feature() { Id = 1, Value = 9.1f },
                new Feature() { Id = 4, Value = 3.6f },
                new Feature() { Id = 14, Value = 11.5f }
            };
        }

        public override Feature[] GetFeatures()
        {
            return features;
        }

        private Feature[] features;
    }

    struct TestInteraction<Ctx>
    { 
        public Ctx Context; 
        public UInt32 Action;
        public float Probability;
        public string UniqueKey;
    }

    class TestContext 
    {
        private int id;

        public int Id
        {
            get { return id; }
            set { id = value; }
        }
    }

    class TestRecorder : IRecorder<TestContext>
    {
        public void Record(TestContext context, UInt32 action, float probability, string uniqueKey)
        {
            interactions.Add(new TestInteraction<TestContext>() { 
                Context = context,
                Action = action,
                Probability = probability,
                UniqueKey = uniqueKey
            });
        }

        public List<TestInteraction<TestContext>> GetAllInteractions()
        {
            return interactions;
        }

        private List<TestInteraction<TestContext>> interactions = new List<TestInteraction<TestContext>>();
    }

    class TestPolicy : IPolicy<TestContext>
    {
        public TestPolicy() : this(-1) { }

        public TestPolicy(int index)
        {
            this.index = index;
        }

        public uint ChooseAction(TestContext context)
        {
            return 5;
        }

        private int index;
    }

    class StringPolicy : IPolicy<SimpleContext>
    {
        public uint ChooseAction(SimpleContext context)
        {
            return 1;
        }
    }

    class TestScorer : IScorer<TestContext>
    {
        public TestScorer(uint numActions, bool uniform = true)
        {
            this.uniform = uniform;
            this.numActions = numActions;
        }
        public List<float> ScoreActions(TestContext context)
        {
            if (uniform)
            {
                return Enumerable.Repeat<float>(1.0f / numActions, (int)numActions).ToList();
            }
            else
            {
                return Array.ConvertAll<int, float>(Enumerable.Range(1, (int)numActions).ToArray(), Convert.ToSingle).ToList();
            }
        }
        private uint numActions;
        private bool uniform;
    }
}

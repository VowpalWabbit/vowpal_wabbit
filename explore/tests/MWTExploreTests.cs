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

            TestRecorder<TestContext> recorder = new TestRecorder<TestContext>();
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

            TestRecorder<TestContext> recorder = new TestRecorder<TestContext>();
            TestPolicy policy = new TestPolicy();
            MwtExplorer<TestContext> mwtt = new MwtExplorer<TestContext>("mwt", recorder);
            TestContext testContext = new TestContext() { Id = 100 };

            var explorer = new TauFirstExplorer<TestContext>(policy, tau, numActions);

            uint expectedAction = policy.ChooseAction(testContext);

            uint chosenAction = mwtt.ChooseAction(explorer, uniqueKey, testContext);
            Assert.AreEqual(expectedAction, chosenAction);

            var interactions = recorder.GetAllInteractions();
            Assert.AreEqual(0, interactions.Count);
        }

        [TestMethod]
        public void Bootstrap()
        {
            uint numActions = 10;
            uint numbags = 2;
            string uniqueKey = "ManagedTestId";

            TestRecorder<TestContext> recorder = new TestRecorder<TestContext>();
            TestPolicy[] policies = new TestPolicy[numbags];
            for (int i = 0; i < numbags; i++)
            {
                policies[i] = new TestPolicy(i * 2);
            }
            TestContext testContext1 = new TestContext() { Id = 99 };
            TestContext testContext2 = new TestContext() { Id = 100 };

            MwtExplorer<TestContext> mwtt = new MwtExplorer<TestContext>("mwt", recorder);
            var explorer = new BootstrapExplorer<TestContext>(policies, numActions);

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
            uint numActionsCover = 100;
            float C = 5;

            TestRecorder<TestContext> recorder = new TestRecorder<TestContext>();
            TestScorer<TestContext> scorer = new TestScorer<TestContext>(numActions);

            MwtExplorer<TestContext> mwtt = new MwtExplorer<TestContext>("mwt", recorder);
            var explorer = new SoftmaxExplorer<TestContext>(scorer, lambda, numActions);

            uint numDecisions = (uint)(numActions * Math.Log(numActions * 1.0) + Math.Log(numActionsCover * 1.0 / numActions) * C * numActions);
            uint[] actions = new uint[numActions];

            Random rand = new Random();
            for (uint i = 0; i < numDecisions; i++)
            {
                uint chosenAction = mwtt.ChooseAction(explorer, rand.NextDouble().ToString(), new TestContext() { Id = (int)i });
                actions[chosenAction - 1]++; // action id is one-based
            }

            for (uint i = 0; i < numActions; i++)
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
            TestRecorder<TestContext> recorder = new TestRecorder<TestContext>();
            TestScorer<TestContext> scorer = new TestScorer<TestContext>(numActions, uniform: false);

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
                Assert.AreNotEqual(interactions[i].Probability, 1.0f / numActions);
                Assert.AreEqual(100 + i, interactions[i].Context.Id);
            }
        }

        [TestMethod]
        public void Generic()
        {
            uint numActions = 10;
            string uniqueKey = "ManagedTestId";
            TestRecorder<TestContext> recorder = new TestRecorder<TestContext>();
            TestScorer<TestContext> scorer = new TestScorer<TestContext>(numActions);

            MwtExplorer<TestContext> mwtt = new MwtExplorer<TestContext>("mwt", recorder);
            var explorer = new GenericExplorer<TestContext>(scorer, numActions);

            TestContext testContext = new TestContext() { Id = 100 };
            uint chosenAction = mwtt.ChooseAction(explorer, uniqueKey, testContext);

            var interactions = recorder.GetAllInteractions();
            Assert.AreEqual(1, interactions.Count);
            Assert.AreEqual(testContext.Id, interactions[0].Context.Id);
        }

        [TestInitialize]
        public void TestInitialize()
        {
        }

        [TestCleanup]
        public void TestCleanup()
        {
        }
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

    class TestRecorder<Ctx> : IRecorder<Ctx>
    {
        public void Record(Ctx context, UInt32 action, float probability, string uniqueKey)
        {
            interactions.Add(new TestInteraction<Ctx>()
            { 
                Context = context,
                Action = action,
                Probability = probability,
                UniqueKey = uniqueKey
            });
        }

        public List<TestInteraction<Ctx>> GetAllInteractions()
        {
            return interactions;
        }

        private List<TestInteraction<Ctx>> interactions = new List<TestInteraction<Ctx>>();
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

    class TestSimplePolicy : IPolicy<SimpleContext>
    {
        public uint ChooseAction(SimpleContext context)
        {
            return 1;
        }
    }

    class StringPolicy : IPolicy<SimpleContext>
    {
        public uint ChooseAction(SimpleContext context)
        {
            return 1;
        }
    }

    class TestScorer<Ctx> : IScorer<Ctx>
    {
        public TestScorer(uint numActions, bool uniform = true)
        {
            this.uniform = uniform;
            this.numActions = numActions;
        }
        public List<float> ScoreActions(Ctx context)
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

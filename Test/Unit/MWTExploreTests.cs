using System;
using System.Runtime.InteropServices;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using MultiWorldTesting.SingleAction;
using System.Collections.Generic;
using System.Linq;
using TestCommon;

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
            var policy = new TestPolicy<RegularTestContext>();
            var testContext = new RegularTestContext();
            var explorer = new EpsilonGreedyExplorer<RegularTestContext>(policy, epsilon, numActions);

            EpsilonGreedyWithContext(numActions, testContext, policy, explorer);
        }

        [TestMethod]
        public void EpsilonGreedyFixedActionUsingVariableActionInterface()
        {
            uint numActions = 10;
            float epsilon = 0f;
            var policy = new TestPolicy<VariableActionTestContext>();
            var testContext = new VariableActionTestContext(numActions);
            var explorer = new EpsilonGreedyExplorer<VariableActionTestContext>(policy, epsilon);

            EpsilonGreedyWithContext(numActions, testContext, policy, explorer);
        }

        private static void EpsilonGreedyWithContext<TContext>(uint numActions, TContext testContext, TestPolicy<TContext> policy, IExplorer<TContext> explorer)
            where TContext : RegularTestContext
        {
            string uniqueKey = "ManagedTestId";
            TestRecorder<TContext> recorder = new TestRecorder<TContext>();
            MwtExplorer<TContext> mwtt = new MwtExplorer<TContext>("mwt", recorder);
            testContext.Id = 100;

            uint expectedAction = policy.ChooseAction(testContext);

            uint chosenAction = mwtt.ChooseAction(explorer, uniqueKey, testContext);
            Assert.AreEqual(expectedAction, chosenAction);

            chosenAction = mwtt.ChooseAction(explorer, uniqueKey, testContext);
            Assert.AreEqual(expectedAction, chosenAction);

            var interactions = recorder.GetAllInteractions();
            Assert.AreEqual(2, interactions.Count);

            Assert.AreEqual(testContext.Id, interactions[0].Context.Id);

            // Verify that policy action is chosen all the time
            explorer.EnableExplore(false);
            for (int i = 0; i < 1000; i++)
            {
                chosenAction = mwtt.ChooseAction(explorer, uniqueKey, testContext);
                Assert.AreEqual(expectedAction, chosenAction);
            }
        }

        [TestMethod]
        public void TauFirst()
        {
            uint numActions = 10;
            uint tau = 0;
            RegularTestContext testContext = new RegularTestContext() { Id = 100 };
            var policy = new TestPolicy<RegularTestContext>();
            var explorer = new TauFirstExplorer<RegularTestContext>(policy, tau, numActions);
            TauFirstWithContext(numActions, testContext, policy, explorer);
        }

        [TestMethod]
        public void TauFirstFixedActionUsingVariableActionInterface()
        {
            uint numActions = 10;
            uint tau = 0;
            var testContext = new VariableActionTestContext(numActions) { Id = 100 };
            var policy = new TestPolicy<VariableActionTestContext>();
            var explorer = new TauFirstExplorer<VariableActionTestContext>(policy, tau);
            TauFirstWithContext(numActions, testContext, policy, explorer);
        }

        private static void TauFirstWithContext<TContext>(uint numActions, TContext testContext, TestPolicy<TContext> policy, IExplorer<TContext> explorer)
            where TContext : RegularTestContext
        {
            string uniqueKey = "ManagedTestId";

            var recorder = new TestRecorder<TContext>();
            var mwtt = new MwtExplorer<TContext>("mwt", recorder);

            uint expectedAction = policy.ChooseAction(testContext);

            uint chosenAction = mwtt.ChooseAction(explorer, uniqueKey, testContext);
            Assert.AreEqual(expectedAction, chosenAction);

            var interactions = recorder.GetAllInteractions();
            Assert.AreEqual(0, interactions.Count);

            // Verify that policy action is chosen all the time
            explorer.EnableExplore(false);
            for (int i = 0; i < 1000; i++)
            {
                chosenAction = mwtt.ChooseAction(explorer, uniqueKey, testContext);
                Assert.AreEqual(expectedAction, chosenAction);
            }
        }

        [TestMethod]
        public void Bootstrap()
        {
            uint numActions = 10;
            uint numbags = 2;
            RegularTestContext testContext1 = new RegularTestContext() { Id = 99 };
            RegularTestContext testContext2 = new RegularTestContext() { Id = 100 };

            var policies = new TestPolicy<RegularTestContext>[numbags];
            for (int i = 0; i < numbags; i++)
            {
                policies[i] = new TestPolicy<RegularTestContext>(i * 2);
            }
            var explorer = new BootstrapExplorer<RegularTestContext>(policies, numActions);

            BootstrapWithContext(numActions, testContext1, testContext2, policies, explorer);
        }

        [TestMethod]
        public void BootstrapFixedActionUsingVariableActionInterface()
        {
            uint numActions = 10;
            uint numbags = 2;
            var testContext1 = new VariableActionTestContext(numActions) { Id = 99 };
            var testContext2 = new VariableActionTestContext(numActions) { Id = 100 };

            var policies = new TestPolicy<VariableActionTestContext>[numbags];
            for (int i = 0; i < numbags; i++)
            {
                policies[i] = new TestPolicy<VariableActionTestContext>(i * 2);
            }
            var explorer = new BootstrapExplorer<VariableActionTestContext>(policies);

            BootstrapWithContext(numActions, testContext1, testContext2, policies, explorer);
        }

        private static void BootstrapWithContext<TContext>(uint numActions, TContext testContext1, TContext testContext2, TestPolicy<TContext>[] policies, IExplorer<TContext> explorer)
            where TContext : RegularTestContext
        {
            string uniqueKey = "ManagedTestId";

            var recorder = new TestRecorder<TContext>();
            var mwtt = new MwtExplorer<TContext>("mwt", recorder);

            uint expectedAction = policies[0].ChooseAction(testContext1);

            uint chosenAction = mwtt.ChooseAction(explorer, uniqueKey, testContext1);
            Assert.AreEqual(expectedAction, chosenAction);

            chosenAction = mwtt.ChooseAction(explorer, uniqueKey, testContext2);
            Assert.AreEqual(expectedAction, chosenAction);

            var interactions = recorder.GetAllInteractions();
            Assert.AreEqual(2, interactions.Count);

            Assert.AreEqual(testContext1.Id, interactions[0].Context.Id);
            Assert.AreEqual(testContext2.Id, interactions[1].Context.Id);

            // Verify that policy action is chosen all the time
            explorer.EnableExplore(false);
            for (int i = 0; i < 1000; i++)
            {
                chosenAction = mwtt.ChooseAction(explorer, uniqueKey, testContext1);
                Assert.AreEqual(expectedAction, chosenAction);
            }
        }

        [TestMethod]
        public void Softmax()
        {
            uint numActions = 10;
            float lambda = 0.5f;
            uint numActionsCover = 100;
            float C = 5;
            var scorer = new TestScorer<RegularTestContext>(1, numActions);
            var explorer = new SoftmaxExplorer<RegularTestContext>(scorer, lambda, numActions);
            
            uint numDecisions = (uint)(numActions * Math.Log(numActions * 1.0) + Math.Log(numActionsCover * 1.0 / numActions) * C * numActions);
            var contexts = new RegularTestContext[numDecisions];
            for (int i = 0; i < numDecisions; i++)
            {
                contexts[i] = new RegularTestContext { Id = i };
            }

            SoftmaxWithContext(numActions, explorer, contexts);
        }

        [TestMethod]
        public void SoftmaxFixedActionUsingVariableActionInterface()
        {
            uint numActions = 10;
            float lambda = 0.5f;
            uint numActionsCover = 100;
            float C = 5;
            var scorer = new TestScorer<VariableActionTestContext>(1, numActions);
            var explorer = new SoftmaxExplorer<VariableActionTestContext>(scorer, lambda);
            
            uint numDecisions = (uint)(numActions * Math.Log(numActions * 1.0) + Math.Log(numActionsCover * 1.0 / numActions) * C * numActions);
            var contexts = new VariableActionTestContext[numDecisions];
            for (int i = 0; i < numDecisions; i++)
            {
                contexts[i] = new VariableActionTestContext(numActions) { Id = i };
            }
            
            SoftmaxWithContext(numActions, explorer, contexts);
        }

        private static void SoftmaxWithContext<TContext>(uint numActions, IExplorer<TContext> explorer, TContext[] contexts)
            where TContext : RegularTestContext
        {
            var recorder = new TestRecorder<TContext>();
            var mwtt = new MwtExplorer<TContext>("mwt", recorder);

            uint[] actions = new uint[numActions];

            Random rand = new Random();
            for (uint i = 0; i < contexts.Length; i++)
            {
                uint chosenAction = mwtt.ChooseAction(explorer, rand.NextDouble().ToString(), contexts[i]);
                actions[chosenAction - 1]++; // action id is one-based
            }

            for (uint i = 0; i < numActions; i++)
            {
                Assert.IsTrue(actions[i] > 0);
            }

            var interactions = recorder.GetAllInteractions();
            Assert.AreEqual(contexts.Length, interactions.Count);

            for (int i = 0; i < contexts.Length; i++)
            {
                Assert.AreEqual(i, interactions[i].Context.Id);
            }
        }

        [TestMethod]
        public void SoftmaxScores()
        {
            uint numActions = 10;
            float lambda = 0.5f;
            var recorder = new TestRecorder<RegularTestContext>();
            var scorer = new TestScorer<RegularTestContext>(1, numActions, uniform: false);

            var mwtt = new MwtExplorer<RegularTestContext>("mwt", recorder);
            var explorer = new SoftmaxExplorer<RegularTestContext>(scorer, lambda, numActions);

            Random rand = new Random();
            mwtt.ChooseAction(explorer, rand.NextDouble().ToString(), new RegularTestContext() { Id = 100 });
            mwtt.ChooseAction(explorer, rand.NextDouble().ToString(), new RegularTestContext() { Id = 101 });
            mwtt.ChooseAction(explorer, rand.NextDouble().ToString(), new RegularTestContext() { Id = 102 });

            var interactions = recorder.GetAllInteractions();
            
            Assert.AreEqual(3, interactions.Count);

            for (int i = 0; i < interactions.Count; i++)
            {
                // Scores are not equal therefore probabilities should not be uniform
                Assert.AreNotEqual(interactions[i].Probability, 1.0f / numActions);
                Assert.AreEqual(100 + i, interactions[i].Context.Id);
            }

            // Verify that policy action is chosen all the time
            RegularTestContext context = new RegularTestContext { Id = 100 };
            List<float> scores = scorer.ScoreActions(context);
            float maxScore = 0;
            uint highestScoreAction = 0;
            for (int i = 0; i < scores.Count; i++)
            {
                if (maxScore < scores[i])
                {
                    maxScore = scores[i];
                    highestScoreAction = (uint)i + 1;
                }
            }

            explorer.EnableExplore(false);
            for (int i = 0; i < 1000; i++)
            {
                uint chosenAction = mwtt.ChooseAction(explorer, rand.NextDouble().ToString(), new RegularTestContext() { Id = (int)i });
                Assert.AreEqual(highestScoreAction, chosenAction);
            }
        }

        [TestMethod]
        public void Generic()
        {
            uint numActions = 10;
            TestScorer<RegularTestContext> scorer = new TestScorer<RegularTestContext>(1, numActions);
            RegularTestContext testContext = new RegularTestContext() { Id = 100 };
            var explorer = new GenericExplorer<RegularTestContext>(scorer, numActions);
            GenericWithContext(numActions, testContext, explorer);
        }

        [TestMethod]
        public void GenericFixedActionUsingVariableActionInterface()
        {
            uint numActions = 10;
            var scorer = new TestScorer<VariableActionTestContext>(1, numActions);
            var testContext = new VariableActionTestContext(numActions) { Id = 100 };
            var explorer = new GenericExplorer<VariableActionTestContext>(scorer);
            GenericWithContext(numActions, testContext, explorer);
        }

        private static void GenericWithContext<TContext>(uint numActions, TContext testContext, IExplorer<TContext> explorer)
            where TContext : RegularTestContext
        {
            string uniqueKey = "ManagedTestId";
            var recorder = new TestRecorder<TContext>();

            var mwtt = new MwtExplorer<TContext>("mwt", recorder);

            uint chosenAction = mwtt.ChooseAction(explorer, uniqueKey, testContext);

            var interactions = recorder.GetAllInteractions();
            Assert.AreEqual(1, interactions.Count);
            Assert.AreEqual(testContext.Id, interactions[0].Context.Id);
        }

        [TestMethod]
        public void UsageBadVariableActionContext()
        {
            int numExceptionsCaught = 0;
            int numExceptionsExpected = 5;

            var tryCatchArgumentException = (Action<Action>)((action) => {
                try
                {
                    action();
                }
                catch (ArgumentException)
                {
                    numExceptionsCaught++;
                }
            });

            tryCatchArgumentException(() => {
                var mwt = new MwtExplorer<RegularTestContext>("test", new TestRecorder<RegularTestContext>());
                var policy = new TestPolicy<RegularTestContext>();
                var explorer = new EpsilonGreedyExplorer<RegularTestContext>(policy, 0.2f);
                mwt.ChooseAction(explorer, "key", new RegularTestContext());
            });
            tryCatchArgumentException(() =>
            {
                var mwt = new MwtExplorer<RegularTestContext>("test", new TestRecorder<RegularTestContext>());
                var policy = new TestPolicy<RegularTestContext>();
                var explorer = new TauFirstExplorer<RegularTestContext>(policy, 10);
                mwt.ChooseAction(explorer, "key", new RegularTestContext());
            });
            tryCatchArgumentException(() =>
            {
                var mwt = new MwtExplorer<RegularTestContext>("test", new TestRecorder<RegularTestContext>());
                var policies = new TestPolicy<RegularTestContext>[2];
                for (int i = 0; i < 2; i++)
                {
                    policies[i] = new TestPolicy<RegularTestContext>(i * 2);
                }
                var explorer = new BootstrapExplorer<RegularTestContext>(policies);
                mwt.ChooseAction(explorer, "key", new RegularTestContext());
            });
            tryCatchArgumentException(() =>
            {
                var mwt = new MwtExplorer<RegularTestContext>("test", new TestRecorder<RegularTestContext>());
                var scorer = new TestScorer<RegularTestContext>(1, 10);
                var explorer = new SoftmaxExplorer<RegularTestContext>(scorer, 0.5f);
                mwt.ChooseAction(explorer, "key", new RegularTestContext());
            });
            tryCatchArgumentException(() =>
            {
                var mwt = new MwtExplorer<RegularTestContext>("test", new TestRecorder<RegularTestContext>());
                var scorer = new TestScorer<RegularTestContext>(1, 10);
                var explorer = new GenericExplorer<RegularTestContext>(scorer);
                mwt.ChooseAction(explorer, "key", new RegularTestContext());
            });

            Assert.AreEqual(numExceptionsExpected, numExceptionsCaught);
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
}

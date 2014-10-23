using System;
using System.Runtime.InteropServices;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using MultiWorldTesting;
using System.Collections.Generic;

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
        public void EpsilonGreedyStateful()
        { 
            mwt.InitializeEpsilonGreedy<int>(Epsilon,
                new StatefulPolicyDelegate<int>(TestStatefulPolicyFunc),
                PolicyParams,
                NumActions);

            uint expectedAction = MWTExploreTests.TestStatefulPolicyFunc(
                PolicyParams,
                context);

            uint chosenAction = mwt.ChooseAction(UniqueKey, context);
            Assert.AreEqual(expectedAction, chosenAction);

            chosenAction = mwt.ChooseAction(UniqueKey, context);
            Assert.AreEqual(expectedAction, chosenAction);

            INTERACTION[] interactions = mwt.GetAllInteractions();
            Assert.AreEqual(2, interactions.Length);

            Assert.AreEqual(2, interactions[0].GetContext().GetFeatures().Length);
            Assert.AreEqual(0.9f, interactions[0].GetContext().GetFeatures()[1].Value);
        }

        [TestMethod]
        public void EpsilonGreedyStateless()
        {
            mwt.InitializeEpsilonGreedy(Epsilon,
                new StatelessPolicyDelegate(TestStatelessPolicyFunc),
                NumActions);

            uint expectedAction = MWTExploreTests.TestStatelessPolicyFunc(context);

            uint chosenAction = mwt.ChooseAction(UniqueKey, context);
            Assert.AreEqual(expectedAction, chosenAction);

            chosenAction = mwt.ChooseAction(UniqueKey, context);
            Assert.AreEqual(expectedAction, chosenAction);

            INTERACTION[] interactions = mwt.GetAllInteractions();
            Assert.AreEqual(2, interactions.Length);

            Assert.AreEqual(2, interactions[0].GetContext().GetFeatures().Length);
            Assert.AreEqual(0.9f, interactions[0].GetContext().GetFeatures()[1].Value);
        }

        [TestMethod]
        public void TauFirstStateful()
        {
            mwt.InitializeTauFirst<int>(Tau,
                new StatefulPolicyDelegate<int>(TestStatefulPolicyFunc),
                PolicyParams,
                NumActions);

            uint expectedAction = MWTExploreTests.TestStatefulPolicyFunc(
                PolicyParams,
                context);

            uint chosenAction = mwt.ChooseAction(UniqueKey, context);
            Assert.AreEqual(expectedAction, chosenAction);

            INTERACTION[] interactions = mwt.GetAllInteractions();
            Assert.AreEqual(0, interactions.Length);
        }

        [TestMethod]
        public void TauFirstStateless()
        {
            mwt.InitializeTauFirst(Tau,
                new StatelessPolicyDelegate(TestStatelessPolicyFunc),
                NumActions);

            uint expectedAction = MWTExploreTests.TestStatelessPolicyFunc(context);

            uint chosenAction = mwt.ChooseAction(UniqueKey, context);
            Assert.AreEqual(expectedAction, chosenAction);

            INTERACTION[] interactions = mwt.GetAllInteractions();
            Assert.AreEqual(0, interactions.Length);
        }

        [TestMethod]
        public void BaggingStateful()
        {
            StatefulPolicyDelegate<int>[] funcs = new StatefulPolicyDelegate<int>[Bags];
            int[] funcParams = new int[Bags];
            for (int i = 0; i < Bags; i++)
			{
                funcs[i] = new StatefulPolicyDelegate<int>(TestStatefulPolicyFunc);
                funcParams[i] = PolicyParams;
			}

            mwt.InitializeBagging(Bags, funcs, funcParams, NumActions);

            uint expectedAction = MWTExploreTests.TestStatefulPolicyFunc(
                PolicyParams,
                context);

            uint chosenAction = mwt.ChooseAction(UniqueKey, context);
            Assert.AreEqual(expectedAction, chosenAction);

            chosenAction = mwt.ChooseAction(UniqueKey, context);
            Assert.AreEqual(expectedAction, chosenAction);

            INTERACTION[] interactions = mwt.GetAllInteractions();
            Assert.AreEqual(2, interactions.Length);

            Assert.AreEqual(2, interactions[0].GetContext().GetFeatures().Length);
            Assert.AreEqual(0.9f, interactions[0].GetContext().GetFeatures()[1].Value);
        }

        [TestMethod]
        public void BaggingStateless()
        {
            StatelessPolicyDelegate[] funcs = new StatelessPolicyDelegate[Bags];
            for (int i = 0; i < Bags; i++)
            {
                funcs[i] = new StatelessPolicyDelegate(TestStatelessPolicyFunc);
            }

            mwt.InitializeBagging(Bags, funcs, NumActions);

            uint expectedAction = MWTExploreTests.TestStatelessPolicyFunc(context);

            uint chosenAction = mwt.ChooseAction(UniqueKey, context);
            Assert.AreEqual(expectedAction, chosenAction);

            chosenAction = mwt.ChooseAction(UniqueKey, context);
            Assert.AreEqual(expectedAction, chosenAction);

            INTERACTION[] interactions = mwt.GetAllInteractions();
            Assert.AreEqual(2, interactions.Length);

            Assert.AreEqual(2, interactions[0].GetContext().GetFeatures().Length);
            Assert.AreEqual(0.9f, interactions[0].GetContext().GetFeatures()[1].Value);
        }

        [TestMethod]
        public void SoftmaxStateful()
        {
            mwt.InitializeSoftmax<int>(Lambda,
                new StatefulScorerDelegate<int>(TestStatefulScorerFunc), 
                PolicyParams, NumActions);

            uint numDecisions = (uint)(NumActions * Math.Log(NumActions * 1.0) + Math.Log(NumActionsCover * 1.0 / NumActions) * C * NumActions);
            uint[] actions = new uint[NumActions];

            Random rand = new Random();
            for (uint i = 0; i < numDecisions; i++)
            {
                uint chosenAction = mwt.ChooseAction(rand.NextDouble().ToString(), context);
                actions[ActionID.Make_ZeroBased(chosenAction)]++;
            }
            
            for (uint i = 0; i < NumActions; i++)
            {
                Assert.IsTrue(actions[i] > 0);
            }

            mwt.GetAllInteractions();
        }

        [TestMethod]
        public void SoftmaxStatefulScores()
        {
            mwt.InitializeSoftmax<int>(Lambda,
                new StatefulScorerDelegate<int>(NonUniformStatefulScorerFunc),
                PolicyParams, NumActions);

            Random rand = new Random();
            mwt.ChooseAction(rand.NextDouble().ToString(), context);
            mwt.ChooseAction(rand.NextDouble().ToString(), context);
            mwt.ChooseAction(rand.NextDouble().ToString(), context);

            INTERACTION[] interactions = mwt.GetAllInteractions();
            for (int i = 0; i < interactions.Length; i++)
            {
                // Scores are not equal therefore probabilities should not be uniform
                Assert.AreNotEqual(interactions[i].GetProbability(), 1.0f / NumActions);
            }
        }

        [TestMethod]
        public void SoftmaxStateless()
        {
            mwt.InitializeSoftmax(Lambda,
                new StatelessScorerDelegate(TestStatelessScorerFunc),
                NumActions);

            uint numDecisions = (uint)(NumActions * Math.Log(NumActions * 1.0) + Math.Log(NumActionsCover * 1.0 / NumActions) * C * NumActions);
            uint[] actions = new uint[NumActions];

            Random rand = new Random();
            for (uint i = 0; i < numDecisions; i++)
            {
                uint chosenAction = mwt.ChooseAction(rand.NextDouble().ToString(), context);
                actions[ActionID.Make_ZeroBased(chosenAction)]++;
            }

            for (uint i = 0; i < NumActions; i++)
            {
                Assert.IsTrue(actions[i] > 0);
            }

            mwt.GetAllInteractions();
        }

        [TestMethod]
        public void SoftmaxStatelessScores()
        {
            mwt.InitializeSoftmax(Lambda,
                new StatelessScorerDelegate(NonUniformStatelessScorerFunc),
                NumActions);

            Random rand = new Random();
            mwt.ChooseAction(rand.NextDouble().ToString(), context);
            mwt.ChooseAction(rand.NextDouble().ToString(), context);
            mwt.ChooseAction(rand.NextDouble().ToString(), context);

            INTERACTION[] interactions = mwt.GetAllInteractions();
            for (int i = 0; i < interactions.Length; i++)
            {
                // Scores are not equal therefore probabilities should not be uniform
                Assert.AreNotEqual(interactions[i].GetProbability(), 1.0f / NumActions);
            }
        }

        [TestMethod]
        public void GenericStateful()
        {
            mwt.InitializeGeneric<int>(
                new StatefulScorerDelegate<int>(TestStatefulScorerFunc),
                PolicyParams,
                NumActions);

            uint chosenAction = mwt.ChooseAction(UniqueKey, context);

            INTERACTION[] interactions = mwt.GetAllInteractions();
            Assert.AreEqual(1, interactions.Length);
            Assert.AreEqual(1.0f / NumActions, interactions[0].GetProbability());
        }
        
        [TestMethod]
        public void GenericStateless()
        {
            mwt.InitializeGeneric(
                new StatelessScorerDelegate(TestStatelessScorerFunc),
                NumActions);

            uint chosenAction = mwt.ChooseAction(UniqueKey, context);

            INTERACTION[] interactions = mwt.GetAllInteractions();
            Assert.AreEqual(1, interactions.Length);
            Assert.AreEqual(1.0f / NumActions, interactions[0].GetProbability());
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

        [TestInitialize]
        public void TestInitialize()
        {
            mwt = new MwtExplorer("test");

            features = new Feature[2];
            features[0].Value = 0.5f;
            features[0].Id = 1;
            features[1].Value = 0.9f;
            features[1].Id = 2;

            context = new CONTEXT(features, "Other C# test context");
        }

        [TestCleanup]
        public void TestCleanup()
        {
            mwt.Unintialize();
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

                CONTEXT c = new CONTEXT(f, null);
                mwt.ChooseAction(i.ToString(), c);

                rewards.Add((float)rand.NextDouble());
            }

            INTERACTION[] partialInteractions = mwt.GetAllInteractions();

            MwtRewardReporter mrr = new MwtRewardReporter(partialInteractions);
            for (int i = 0; i < partialInteractions.Length; i++)
            {
                Assert.AreEqual(true, mrr.ReportReward(partialInteractions[i].GetId(), rewards[i]));
            }

            INTERACTION[] completeInteractions = mrr.GetAllInteractions();
            MwtOptimizer mop = new MwtOptimizer(completeInteractions, NumActions);

            string modelFile = "model";

            mop.OptimizePolicyVWCSOAA(modelFile);

            Assert.IsTrue(System.IO.File.Exists(modelFile));

            float evaluatedValue = mop.EvaluatePolicyVWCSOAA(modelFile);

            Assert.IsFalse(float.IsNaN(evaluatedValue));

            System.IO.File.Delete(modelFile);
        }

        private static UInt32 TestStatefulPolicyFunc(int policyParams, CONTEXT context)
        {
            return ActionID.Make_OneBased((uint)(policyParams + context.GetFeatures().Length) % MWTExploreTests.NumActions);
        }

        private static UInt32 TestStatelessPolicyFunc(CONTEXT context)
        {
            return ActionID.Make_OneBased((uint)context.GetFeatures().Length % MWTExploreTests.NumActions);
        }

        private static void TestStatefulScorerFunc(int policyParams, CONTEXT applicationContext, float[] scores)
        {
            for (uint i = 0; i < scores.Length; i++)
            {
                scores[i] = policyParams;
            }
        }

        private static void TestStatelessScorerFunc(CONTEXT applicationContext, float[] scores)
        {
            for (uint i = 0; i < scores.Length; i++)
            {
                scores[i] = applicationContext.GetFeatures().Length;
            }
        }

        private static void NonUniformStatefulScorerFunc(int policyParams, CONTEXT applicationContext, float[] scores)
        {
            for (uint i = 0; i < scores.Length; i++)
            {
                scores[i] = policyParams + i;
            }
        }

        private static void NonUniformStatelessScorerFunc(CONTEXT applicationContext, float[] scores)
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

        private MwtExplorer mwt;
        private Feature[] features;
        private CONTEXT context;
    }
}

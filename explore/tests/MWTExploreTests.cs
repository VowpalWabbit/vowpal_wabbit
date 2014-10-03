using System;
using System.Runtime.InteropServices;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using MultiWorldTesting;

namespace ExploreTests
{
    [TestClass]
    public class MWTExploreTests
    {
        [TestMethod]
        public void EpsilonGreedyStateful()
        {
            mwt.InitializeEpsilonGreedy(Epsilon, 
                new StatefulPolicyDelegate(TestStatefulPolicyFunc),
                new IntPtr(PolicyParams),
                NumActions);

            uint expectedAction = MWTExploreTests.TestStatefulPolicyFunc(
                new IntPtr(PolicyParams),
                (IntPtr)contextHandle);

            uint chosenAction = mwt.ChooseAction(context, UniqueKey);
            Assert.AreEqual(expectedAction, chosenAction);
        }

        [TestInitialize]
        public void TestInitialize()
        {
            mwt = new MWTWrapper("MWTManagedTests");

            features = new FEATURE[2];
            features[0].X = 0.5f;
            features[0].WeightIndex = 1;
            features[1].X = 0.9f;
            features[1].WeightIndex = 2;

            context = new Context(features, "Other C# test context");
            contextHandle = GCHandle.Alloc(context);
        }

        [TestCleanup]
        public void TestCleanup()
        {
            contextHandle.Free();
        }

        private static UInt32 TestStatelessPolicyFunc(IntPtr applicationContext)
        {
            GCHandle contextHandle = (GCHandle)applicationContext;
            Context context = (contextHandle.Target as Context);
            
            return (uint)context.Features.Length % MWTExploreTests.NumActions + 1;
        }

        private static UInt32 TestStatefulPolicyFunc(IntPtr policyParams, IntPtr applicationContext)
        {
            GCHandle contextHandle = (GCHandle)applicationContext;
            Context context = (contextHandle.Target as Context);

            return (uint)(policyParams + context.Features.Length) % MWTExploreTests.NumActions + 1;
        }

        private static void TestStatefulScorerFunc(IntPtr policyParams, IntPtr applicationContext, float[] scores, uint size)
        {
            for (uint i = 0; i < size; i++)
            {
                scores[i] = (int)policyParams + i;
            }
        }
        private static void TestStatelessScorerFunc(IntPtr applicationContext, float[] scores, uint size)
        {
            for (uint i = 0; i < size; i++)
            {
                scores[i] = i;
            }
        }

        private static readonly uint NumActions = 10;
        private static readonly float Epsilon = 0.2f;
        private static readonly uint Tau = 5;
        private static readonly uint Bags = 2;
        private static readonly float Lambda = 0.5f;
        private static readonly int PolicyParams = 1003;
        private static readonly string UniqueKey = "ManagedTestId";

        private MWTWrapper mwt;
        private FEATURE[] features;
        private Context context;
        private GCHandle contextHandle;
    }
}

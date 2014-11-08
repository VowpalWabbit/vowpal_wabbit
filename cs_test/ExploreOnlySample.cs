using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using MultiWorldTesting;

namespace cs_test
{
    class ExploreOnlySample
    {
        private static UInt32 SampleStatefulPolicyFunc(int policyParams, BaseContext appContext)
        {
            return (uint)((policyParams + appContext.GetFeatures().Length) % 10 + 1);
        }

        private static UInt32 SampleStatefulPolicyFunc2(int policyParams, BaseContext appContext)
        {
            return (uint)((policyParams + appContext.GetFeatures().Length) % 10 + 2);
        }

        private static UInt32 SampleStatefulPolicyFunc(CustomParams policyParams, BaseContext appContext)
        {
            return (uint)((policyParams.Value1 + policyParams.Value2 + appContext.GetFeatures().Length) % 10 + 1);
        }

        private static UInt32 SampleStatelessPolicyFunc(BaseContext appContext)
        {
            return (UInt32)appContext.GetFeatures().Length;
        }

        private static UInt32 SampleStatelessPolicyFunc2(BaseContext appContext)
        {
            return (UInt32)appContext.GetFeatures().Length + 1;
        }

        private static void SampleStatefulScorerFunc(int policyParams, BaseContext appContext, float[] scores)
        {
            for (uint i = 0; i < scores.Length; i++)
            {
                scores[i] = (int)policyParams + i;
            }
        }

        private static void SampleStatelessScorerFunc(BaseContext appContext, float[] scores)
        {
            for (uint i = 0; i < scores.Length; i++)
            {
                scores[i] = appContext.GetFeatures().Length + i;
            }
        }

        class CustomParams
        {
            public int Value1;
            public int Value2;
        }

        class MyContext { }
        class MyRecorder : IRecorder<MyContext>
        {
            public void Record(MyContext context, UInt32 action, float probability, string uniqueKey)
            {
                actions.Add(action);
            }
            private List<uint> actions = new List<uint>();
        }
        class MyPolicy : IPolicy<MyContext>
        {
            public uint ChooseAction(MyContext context)
            {
                return 5;
            }
        }

        public static void Run()
        {
            string exploration_type = "greedy";
            bool stateful = true;

            MwtExplorer mwt = new MwtExplorer("test");

            uint numActions = 10;

            float epsilon = 0.2f;
            uint tau = 0;
            uint numbags = 2;
            float lambda = 0.5f;

            int policyParams = 1003;
            int policyParams2 = 1004;
            CustomParams customParams = new CustomParams() { Value1 = policyParams, Value2 = policyParams2 };
            StatefulPolicyDelegate<int>[] bags = 
            {
                  new StatefulPolicyDelegate<int>(SampleStatefulPolicyFunc), 
                  new StatefulPolicyDelegate<int>(SampleStatefulPolicyFunc2) 
            };
            int[] parameters = { policyParams, policyParams };
            StatelessPolicyDelegate[] statelessbags = 
            {
                new StatelessPolicyDelegate(SampleStatelessPolicyFunc), 
                new StatelessPolicyDelegate(SampleStatelessPolicyFunc2) 
            };

            if (exploration_type == "greedy")
            {
                MyRecorder mc = new MyRecorder();
                MyPolicy mp = new MyPolicy();
                MWT<MyContext> mwtt = new MWT<MyContext>("mwt", mc);
                mwtt.Choose_Action(new EpsilonGreedyExplorer<MyContext>(mp, epsilon, numActions), "key", new MyContext());
                return;
            }
            else if (exploration_type == "tau-first")
            {
                if (stateful)
                {
                    /*** Initialize Tau-First explore algorithm using a default policy function that accepts parameters ***/
                    mwt.InitializeTauFirst<CustomParams>(tau, new StatefulPolicyDelegate<CustomParams>(SampleStatefulPolicyFunc), customParams, numActions);
                }
                else
                {
                    /*** Initialize Tau-First explore algorithm using a stateless default policy function ***/
                    mwt.InitializeTauFirst(tau, new StatelessPolicyDelegate(SampleStatelessPolicyFunc), numActions);
                }
            }
            else if (exploration_type == "bagging")
            {
                if (stateful)
                {
                    /*** Initialize Bagging explore algorithm using a default policy function that accepts parameters ***/
                    mwt.InitializeBagging<int>(numbags, bags, parameters, numActions);
                }
                else
                {

                    /*** Initialize Bagging explore algorithm using a stateless default policy function ***/
                    mwt.InitializeBagging(numbags, statelessbags, numActions);
                }
            }
            else if (exploration_type == "softmax")
            {
                if (stateful)
                {
                    /*** Initialize Softmax explore algorithm using a default policy function that accepts parameters ***/
                    mwt.InitializeSoftmax<int>(lambda, new StatefulScorerDelegate<int>(SampleStatefulScorerFunc), policyParams, numActions);
                }
                else
                {
                    /*** Initialize Softmax explore algorithm using a stateless default policy function ***/
                    mwt.InitializeSoftmax(lambda, new StatelessScorerDelegate(SampleStatelessScorerFunc), numActions);
                }
            }
            else if (exploration_type == "generic")
            {
                if (stateful)
                {
                    /*** Initialize Generic explore algorithm using a default policy function that accepts parameters ***/
                    mwt.InitializeGeneric<int>(new StatefulScorerDelegate<int>(SampleStatefulScorerFunc), policyParams, numActions);
                }
                else
                {
                    /*** Initialize Generic explore algorithm using a stateless default policy function ***/
                    mwt.InitializeGeneric(new StatelessScorerDelegate(SampleStatelessScorerFunc), numActions);
                }
            }
            else
            {  //add error here


            }
            Feature[] f = new Feature[2];
            f[0].Value = 0.5f;
            f[0].Id = 1;
            f[1].Value = 0.9f;
            f[1].Id = 2;

            string otherContext = "Some other context data that might be helpful to log";
            OldSimpleContext appContext = new OldSimpleContext(f, otherContext);

            UInt32 chosenAction = mwt.ChooseAction("myId", appContext);

            Interaction[] interactions = mwt.GetAllInteractions();
            // string interactions = mwt.GetAllInteractionsAsString();

            mwt.Uninitialize();

            Console.WriteLine(chosenAction);
            Console.WriteLine(interactions);

        }
    }
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using MultiWorldTesting;

namespace cs_test
{
    class VWExploreTests
    {
        private static UInt32 MyStatelessPolicyFunc(IntPtr applicationContext)
        {
            return 222;
        }

        private static UInt32 MyStatefulPolicyFunc(IntPtr policyParams, IntPtr applicationContext)
        {
            return 111;
        }

        private static void MyStatefulScorerFunc(IntPtr policyParams, IntPtr applicationContext, float[] scores, uint size)
        {
            for (uint i = 0; i < size; i++)
            {
                scores[i] = (int)policyParams + i;
            }
        }
        private static void MyStatelessScorerFunc(IntPtr applicationContext, float[] scores, uint size)
        {
            for (uint i = 0; i < size; i++)
            {
                scores[i] = i;
            }
        }

        public static void RunMWTExploreTest()
        {
            MWTWrapper mwt = new MWTWrapper("myTestApp");

            uint numActions = 10;
            
            float epsilon = 0.2f;
            uint tau = 5;
            uint bags = 10;
            float lambda = 0.5f;

            int policyParams = 1003;

            /*** Initialize Epsilon-Greedy explore algorithm using a default policy function that accepts parameters ***/
            mwt.InitializeEpsilonGreedy(epsilon, new StatefulPolicyDelegate(MyStatefulPolicyFunc), new IntPtr(policyParams), numActions);

            /*** Initialize Epsilon-Greedy explore algorithm using a stateless default policy function ***/
            //mwt.InitializeEpsilonGreedy(epsilon, new StatelessPolicyDelegate(MyStatelessPolicyFunc), numActions);

            /*** Initialize Tau-First explore algorithm using a default policy function that accepts parameters ***/
            //mwt.InitializeTauFirst(tau, new StatefulPolicyDelegate(MyStatefulPolicyFunc), new IntPtr(policyParams), numActions);

            /*** Initialize Tau-First explore algorithm using a stateless default policy function ***/
            //mwt.InitializeTauFirst(tau, new StatelessPolicyDelegate(MyStatelessPolicyFunc), numActions);

            /*** Initialize Bagging explore algorithm using a default policy function that accepts parameters ***/
            //StatefulPolicyDelegate[] funcs = 
            //{
            //    new StatefulPolicyDelegate(MyStatefulPolicyFunc), 
            //    new StatefulPolicyDelegate(MyStatefulPolicyFunc) 
            //};
            //IntPtr[] parameters = { new IntPtr(policyParams), new IntPtr(policyParams) };
            //mwt.InitializeBagging(bags, funcs, parameters, numActions);

            /*** Initialize Bagging explore algorithm using a stateless default policy function ***/
            //StatelessPolicyDelegate[] funcs = 
            //{
            //    new StatelessPolicyDelegate(MyStatelessPolicyFunc), 
            //    new StatelessPolicyDelegate(MyStatelessPolicyFunc) 
            //};
            //mwt.InitializeBagging(bags, funcs, numActions);

            /*** Initialize Softmax explore algorithm using a default policy function that accepts parameters ***/
            //mwt.InitializeSoftmax(lambda, new StatefulScorerDelegate(MyStatefulScorerFunc), new IntPtr(policyParams), numActions);

            /*** Initialize Softmax explore algorithm using a stateless default policy function ***/
            //mwt.InitializeSoftmax(lambda, new StatelessScorerDelegate(MyStatelessScorerFunc), numActions);

            FEATURE[] f = new FEATURE[2];
            f[0].X = 0.5f;
            f[0].WeightIndex = 1;
            f[1].X = 0.9f;
            f[1].WeightIndex = 2;

            string otherContext = "Some other context data that might be helpful to log";
            CONTEXT context = new CONTEXT(f, otherContext);

            UInt32 chosenAction = mwt.ChooseAction(context, "myId");

            string interactions = mwt.GetAllInteractions();

            Console.WriteLine(chosenAction);
            Console.WriteLine(interactions);
        }
    }
}

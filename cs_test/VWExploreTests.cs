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

        private static UInt32 MyStatefulPolicyFunc(IntPtr stateContext, IntPtr applicationContext)
        {
            return 111;
        }

        public static void RunMWTExploreTest()
        {
            MWTWrapper mwt = new MWTWrapper("myTestApp", 10);

            if (true)
            {
                mwt.InitializeEpsilonGreedy(0.5f, new StatelessPolicyDelegate(MyStatelessPolicyFunc));
            }
            else
            {
                mwt.InitializeEpsilonGreedy(0.5f, new StatefulPolicyDelegate(MyStatefulPolicyFunc), new IntPtr(1003));
            }

            FEATURE[] f = new FEATURE[2];
            f[0].X = 0.5f;
            f[0].WeightIndex = 1;
            f[1].X = 0.9f;
            f[1].WeightIndex = 2;

            UInt32 chosenAction = mwt.ChooseAction(f, "saywhathahahahah", "myId");

            string interactions = mwt.GetAllInteractions();

            Console.WriteLine(chosenAction);
            Console.WriteLine(interactions);
        }
    }
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using MultiWorldTesting;

namespace cs_test
{
    class ExploreOnlySample
    {
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
            public MyPolicy() : this(-1) { }

            public MyPolicy(int index)
            {
                this.index = index;
            }

            public uint ChooseAction(MyContext context)
            {
                return 5;
            }

            private int index;
        }

        class MyScorer : IScorer<MyContext>
        {
            public MyScorer(uint numActions)
            {
                this.numActions = numActions;
            }
            public List<float> ScoreActions(MyContext context)
            {
                return Enumerable.Repeat<float>(1.0f / numActions, (int)numActions).ToList();
            }
            private uint numActions;
        }

        public static void Run()
        {
            string exploration_type = "generic";

            if (exploration_type == "greedy")
            {
                // Initialize Epsilon-Greedy explore algorithm using custom Recorder, Policy & Context types
                uint numActions = 10;
                float epsilon = 0.2f;
                MyRecorder mc = new MyRecorder();
                MyPolicy mp = new MyPolicy();
                MWT<MyContext> mwtt = new MWT<MyContext>("mwt", mc);
                uint action = mwtt.Choose_Action(new EpsilonGreedyExplorer<MyContext>(mp, epsilon, numActions), "key", new MyContext());
                return;
            }
            else if (exploration_type == "tau-first")
            {
                // Initialize Tau-First explore algorithm using custom Recorder, Policy & Context types
                uint numActions = 10;
                uint tau = 0;
                MyRecorder mc = new MyRecorder();
                MyPolicy mp = new MyPolicy();
                MWT<MyContext> mwtt = new MWT<MyContext>("mwt", mc);
                uint action = mwtt.Choose_Action(new TauFirstExplorer<MyContext>(mp, tau, numActions), "key", new MyContext());
                return;
            }
            else if (exploration_type == "bagging")
            {
                // Initialize Bagging explore algorithm using custom Recorder, Policy & Context types
                uint numActions = 10;
                uint numbags = 2;
                MyRecorder mc = new MyRecorder();
                MyPolicy[] mps = new MyPolicy[numbags];
                for (int i = 0; i < numbags; i++)
                {
                    mps[i] = new MyPolicy(i * 2);
                }

                MWT<MyContext> mwtt = new MWT<MyContext>("mwt", mc);
                uint action = mwtt.Choose_Action(new BaggingExplorer<MyContext>(mps, numbags, numActions), "key", new MyContext());
                return;
            }
            else if (exploration_type == "softmax")
            {
                // Initialize Softmax explore algorithm using custom Recorder, Scorer & Context types
                uint numActions = 10;
                float lambda = 0.5f;
                MyRecorder mc = new MyRecorder();
                MyScorer ms = new MyScorer(numActions);

                MWT<MyContext> mwtt = new MWT<MyContext>("mwt", mc);
                uint action = mwtt.Choose_Action(new SoftmaxExplorer<MyContext>(ms, lambda, numActions), "key", new MyContext());
                return;
            }
            else if (exploration_type == "generic")
            {
                // Initialize Generic explore algorithm using custom Recorder, Scorer & Context types
                uint numActions = 10;
                MyRecorder mc = new MyRecorder();
                MyScorer ms = new MyScorer(numActions);

                MWT<MyContext> mwtt = new MWT<MyContext>("mwt", mc);
                uint action = mwtt.Choose_Action(new GenericExplorer<MyContext>(ms, numActions), "key", new MyContext());
                return;
            }
            else
            {  //add error here


            }
        }
    }
}

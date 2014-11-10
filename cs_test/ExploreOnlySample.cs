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

            public List<uint> GetData()
            {
                return actions;
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

        class StringPolicy : IPolicy<SimpleContext>
        {
            public uint ChooseAction(SimpleContext context)
            {
                return 1;
            }
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
            string exploration_type = "greedy";

            if (exploration_type == "greedy")
            {
                // Initialize Epsilon-Greedy explore algorithm using built-in StringRecorder and SimpleContext types
                uint numActions = 10;
                float epsilon = 0.2f;

                SimpleContext context = new SimpleContext(new Feature[] { 
                    new Feature() { Id = 1, Value = 0.5f },
                    new Feature() { Id = 4, Value = 1.3f },
                    new Feature() { Id = 9, Value = -0.5f },
                });
                StringRecorder<SimpleContext> recorder = new StringRecorder<SimpleContext>();
                StringPolicy policy = new StringPolicy();
                MwtExplorer<SimpleContext> mwtt = new MwtExplorer<SimpleContext>("mwt", recorder);
                uint action = mwtt.ChooseAction(new EpsilonGreedyExplorer<SimpleContext>(policy, epsilon, numActions), "key", context);

                Console.WriteLine(recorder.GetRecording());

                return;
            }
            else if (exploration_type == "tau-first")
            {
                // Initialize Tau-First explore algorithm using custom Recorder, Policy & Context types
                uint numActions = 10;
                uint tau = 0;
                MyRecorder recorder = new MyRecorder();
                MyPolicy policy = new MyPolicy();
                MwtExplorer<MyContext> mwtt = new MwtExplorer<MyContext>("mwt", recorder);
                uint action = mwtt.ChooseAction(new TauFirstExplorer<MyContext>(policy, tau, numActions), "key", new MyContext());
                Console.WriteLine(String.Join(",", recorder.GetData()));
                return;
            }
            else if (exploration_type == "bagging")
            {
                // Initialize Bagging explore algorithm using custom Recorder, Policy & Context types
                uint numActions = 10;
                uint numbags = 2;
                MyRecorder recorder = new MyRecorder();
                MyPolicy[] policies = new MyPolicy[numbags];
                for (int i = 0; i < numbags; i++)
                {
                    policies[i] = new MyPolicy(i * 2);
                }

                MwtExplorer<MyContext> mwtt = new MwtExplorer<MyContext>("mwt", recorder);
                uint action = mwtt.ChooseAction(new BaggingExplorer<MyContext>(policies, numbags, numActions), "key", new MyContext());
                Console.WriteLine(String.Join(",", recorder.GetData()));
                return;
            }
            else if (exploration_type == "softmax")
            {
                // Initialize Softmax explore algorithm using custom Recorder, Scorer & Context types
                uint numActions = 10;
                float lambda = 0.5f;
                MyRecorder recorder = new MyRecorder();
                MyScorer scorer = new MyScorer(numActions);

                MwtExplorer<MyContext> mwtt = new MwtExplorer<MyContext>("mwt", recorder);
                uint action = mwtt.ChooseAction(new SoftmaxExplorer<MyContext>(scorer, lambda, numActions), "key", new MyContext());
                Console.WriteLine(String.Join(",", recorder.GetData()));
                return;
            }
            else if (exploration_type == "generic")
            {
                // Initialize Generic explore algorithm using custom Recorder, Scorer & Context types
                uint numActions = 10;
                MyRecorder recorder = new MyRecorder();
                MyScorer scorer = new MyScorer(numActions);

                MwtExplorer<MyContext> mwtt = new MwtExplorer<MyContext>("mwt", recorder);
                uint action = mwtt.ChooseAction(new GenericExplorer<MyContext>(scorer, numActions), "key", new MyContext());
                Console.WriteLine(String.Join(",", recorder.GetData()));
                return;
            }
            else
            {  //add error here


            }
        }
    }
}

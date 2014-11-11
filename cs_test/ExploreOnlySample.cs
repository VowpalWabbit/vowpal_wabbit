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
                interactions.Add(new Interaction<MyContext>() {
                    Context = context,
                    Action = action,
                    Probability = probability,
                    UniqueKey = uniqueKey
                });
            }

            public List<Interaction<MyContext>> GetAllInteractions()
            {
                return interactions;
            }

            private List<Interaction<MyContext>> interactions = new List<Interaction<MyContext>>();
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

        struct Interaction<Ctx>
        {
            public Ctx Context;
            public uint Action;
            public float Probability;
            public string UniqueKey;
        }

        public static void Run()
        {
            string exploration_type = "greedy";

            if (exploration_type == "greedy")
            {
                // Initialize Epsilon-Greedy explore algorithm using built-in StringRecorder and SimpleContext types
                StringRecorder<SimpleContext> recorder = new StringRecorder<SimpleContext>();
                MwtExplorer<SimpleContext> mwtt = new MwtExplorer<SimpleContext>("mwt", recorder);

                uint numActions = 10;
                float epsilon = 0.2f;
                StringPolicy policy = new StringPolicy();
                SimpleContext context = new SimpleContext(new Feature[] { 
                    new Feature() { Id = 1, Value = 0.5f },
                    new Feature() { Id = 4, Value = 1.3f },
                    new Feature() { Id = 9, Value = -0.5f },
                });
                uint action = mwtt.ChooseAction(new EpsilonGreedyExplorer<SimpleContext>(policy, epsilon, numActions), "key", context);

                Console.WriteLine(recorder.FlushRecording());

                return;
            }
            else if (exploration_type == "tau-first")
            {
                // Initialize Tau-First explore algorithm using custom Recorder, Policy & Context types
                MyRecorder recorder = new MyRecorder();
                MwtExplorer<MyContext> mwtt = new MwtExplorer<MyContext>("mwt", recorder);

                uint numActions = 10;
                uint tau = 0;
                MyPolicy policy = new MyPolicy();
                uint action = mwtt.ChooseAction(new TauFirstExplorer<MyContext>(policy, tau, numActions), "key", new MyContext());
                Console.WriteLine(String.Join(",", recorder.GetAllInteractions().Select(it => it.Action)));
                return;
            }
            else if (exploration_type == "bootstrap")
            {
                // Initialize Bootstrap explore algorithm using custom Recorder, Policy & Context types
                MyRecorder recorder = new MyRecorder();
                MwtExplorer<MyContext> mwtt = new MwtExplorer<MyContext>("mwt", recorder);

                uint numActions = 10;
                uint numbags = 2;
                MyPolicy[] policies = new MyPolicy[numbags];
                for (int i = 0; i < numbags; i++)
                {
                    policies[i] = new MyPolicy(i * 2);
                }
                uint action = mwtt.ChooseAction(new BootstrapExplorer<MyContext>(policies, numActions), "key", new MyContext());
                Console.WriteLine(String.Join(",", recorder.GetAllInteractions().Select(it => it.Action)));
                return;
            }
            else if (exploration_type == "softmax")
            {
                // Initialize Softmax explore algorithm using custom Recorder, Scorer & Context types
                MyRecorder recorder = new MyRecorder();
                MwtExplorer<MyContext> mwtt = new MwtExplorer<MyContext>("mwt", recorder);

                uint numActions = 10;
                float lambda = 0.5f;
                MyScorer scorer = new MyScorer(numActions);
                uint action = mwtt.ChooseAction(new SoftmaxExplorer<MyContext>(scorer, lambda, numActions), "key", new MyContext());

                Console.WriteLine(String.Join(",", recorder.GetAllInteractions().Select(it => it.Action)));
                return;
            }
            else if (exploration_type == "generic")
            {
                // Initialize Generic explore algorithm using custom Recorder, Scorer & Context types
                MyRecorder recorder = new MyRecorder();
                MwtExplorer<MyContext> mwtt = new MwtExplorer<MyContext>("mwt", recorder);

                uint numActions = 10;
                MyScorer scorer = new MyScorer(numActions);
                uint action = mwtt.ChooseAction(new GenericExplorer<MyContext>(scorer, numActions), "key", new MyContext());

                Console.WriteLine(String.Join(",", recorder.GetAllInteractions().Select(it => it.Action)));
                return;
            }
            else
            {  //add error here


            }
        }
    }
}

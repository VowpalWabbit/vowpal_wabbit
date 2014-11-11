using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using MultiWorldTesting;

namespace cs_test
{
    class ExploreOnlySample
    {
        /// <summary>
        /// Example of a custom context.
        /// </summary>
        class MyContext { }

        /// <summary>
        /// Example of a custom recorder which implements the IRecorder<MyContext>,
        /// declaring that this recorder only interacts with MyContext objects.
        /// </summary>
        class MyRecorder : IRecorder<MyContext>
        {
            public void Record(MyContext context, UInt32 action, float probability, string uniqueKey)
            {
                // Stores the tuple internally in a vector that could be used later for other purposes.
                interactions.Add(new Interaction<MyContext>()
                {
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

        /// <summary>
        /// Example of a custom policy which implements the IPolicy<MyContext>,
        /// declaring that this policy only interacts with MyContext objects.
        /// </summary>
        class MyPolicy : IPolicy<MyContext>
        {
            public MyPolicy() : this(-1) { }

            public MyPolicy(int index)
            {
                this.index = index;
            }

            public uint ChooseAction(MyContext context)
            {
                // Always returns the same action regardless of context
                return 5;
            }

            private int index;
        }

        /// <summary>
        /// Example of a custom policy which implements the IPolicy<SimpleContext>,
        /// declaring that this policy only interacts with SimpleContext objects.
        /// </summary>
        class StringPolicy : IPolicy<SimpleContext>
        {
            public uint ChooseAction(SimpleContext context)
            {
                // Always returns the same action regardless of context
                return 1;
            }
        }

        /// <summary>
        /// Example of a custom scorer which implements the IScorer<MyContext>,
        /// declaring that this scorer only interacts with MyContext objects.
        /// </summary>
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

        /// <summary>
        /// Represents a tuple <context, action, probability, key>.
        /// </summary>
        /// <typeparam name="Ctx">The Context type.</typeparam>
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
                
                // Creates a recorder of built-in StringRecorder type for string serialization
                StringRecorder<SimpleContext> recorder = new StringRecorder<SimpleContext>();
                
                // Creates an MwtExplorer instance using the recorder above
                MwtExplorer<SimpleContext> mwtt = new MwtExplorer<SimpleContext>("mwt", recorder);

		        // Creates a policy that interacts with SimpleContext type
                StringPolicy policy = new StringPolicy();

                uint numActions = 10;
                float epsilon = 0.2f;
		        // Creates an Epsilon-Greedy explorer using the specified settings
                EpsilonGreedyExplorer<SimpleContext> explorer = new EpsilonGreedyExplorer<SimpleContext>(policy, epsilon, numActions);

                // Creates a context of built-in SimpleContext type
                SimpleContext context = new SimpleContext(new Feature[] { 
                    new Feature() { Id = 1, Value = 0.5f },
                    new Feature() { Id = 4, Value = 1.3f },
                    new Feature() { Id = 9, Value = -0.5f },
                });

                // Performs exploration by passing an instance of the Epsilon-Greedy exploration algorithm into MwtExplorer
                // using a sample string to uniquely identify this event
                string uniqueKey = "eventid";
                uint action = mwtt.ChooseAction(explorer, uniqueKey, context);

                Console.WriteLine(recorder.GetRecording());

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

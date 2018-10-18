using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using Rl.Net;

namespace Rl.Net.Cli
{
    public enum Topic : long
    {
        HerbGarden,
        MachineLearning
    }

    internal static class ActionDistributionExtensions
    {
        public static string ToDistributionString(this IEnumerable<ActionProbability> actionDistribution)
        {
            StringBuilder stringBuilder = new StringBuilder("(");

            foreach (ActionProbability actionProbability in actionDistribution)
            {
                stringBuilder.Append($"[{actionProbability.ActionIndex}, {actionProbability.Probability}]");
            }

            stringBuilder.Append(')');

            return stringBuilder.ToString();
        }
    }

    internal class SimulatorStepProvider : IDriverStepProvider<float>
    {
        public static readonly Random RandomSource = new Random();
        public const int InfinitySteps = -1;

        private static Func<Topic, float> GenerateRewardDistribution(float herbGardenProbability, float machineLearningProbability)
        {
            Dictionary<Topic, float> topicProbabilities = new Dictionary<Topic, float>
            {
                { Topic.HerbGarden, herbGardenProbability },
                { Topic.MachineLearning, machineLearningProbability }
            };

            return (topic) => topicProbabilities[topic];
        }

        internal static Person[] People = new[]
        {
            new Person("rnc", "engineering", "hiking", "spock", GenerateRewardDistribution(0.03f, 0.1f)),
            new Person("mk", "psychology", "kids", "7of9", GenerateRewardDistribution(0.3f, 0.1f))
        };

        private static Person GetRandomPerson()
        {
            int index = RandomSource.Next(People.Length);

            return People[index];
        }

        private readonly int steps;

        public SimulatorStepProvider(int steps)
        {
            this.steps = steps;
        }

        public IEnumerator<IStepContext<float>> GetEnumerator()
        {
            StatisticsCalculator stats = new StatisticsCalculator();

            int stepsSoFar = 0;
            while (steps < 0 || (stepsSoFar++ < steps))
            {
                SimulatorStep step = new SimulatorStep
                {
                    StatisticsCalculator = stats,
                    EventId = Guid.NewGuid().ToString(),
                    Person = GetRandomPerson()
                };

                yield return step;

                step.Record(stats);
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return this.GetEnumerator();
        }


        internal class SimulatorStep : IStepContext<float>
        {
            internal static readonly Topic[] ActionSet = new[] { Topic.HerbGarden, Topic.MachineLearning };

            private static readonly string ActionsJson = string.Join(",", ActionSet.Select(topic => $"{{ \"TAction\": {{ \"topic\": \"{topic}\" }} }}"));

            public StatisticsCalculator StatisticsCalculator
            {
                get;
                set;
            }

            public string EventId
            {
                get;
                set;
            }

            public Person Person
            {
                get;
                set;
            }

            public Topic? DecisionCache
            {
                get;
                set;
            }

            public string ActionDistributionString
            {
                get
                {
                    if (this.actionDistributionCache == null)
                    {
                        return null;
                    }

                    return this.actionDistributionCache.ToDistributionString();
                }
            }

            public string DecisionContext => $"{{ { this.Person.FeaturesJson }, \"_multi\": [{ ActionsJson }] }}";

            private float? outcomeCache;
            private IEnumerable<ActionProbability> actionDistributionCache;
            public float GetOutcome(long actionIndex, IEnumerable<ActionProbability> actionDistribution)
            {
                if (!this.outcomeCache.HasValue)
                {
                    this.DecisionCache = (Topic)actionIndex;
                    this.actionDistributionCache = actionDistribution;
                    this.outcomeCache = this.Person.GenerateOutcome(this.DecisionCache.Value);
                }

                return this.outcomeCache.Value;
            }

            public void Record(StatisticsCalculator statisticsCalculator)
            {
                statisticsCalculator.Record(this.Person, this.DecisionCache.Value, this.outcomeCache.Value);

                Console.WriteLine($" {statisticsCalculator.TotalActions}, ctxt, {this.Person.Id}, action, {this.DecisionCache.Value}, outcome, {this.outcomeCache.Value}, dist, {this.ActionDistributionString}, {statisticsCalculator.GetStats(this.Person, this.DecisionCache.Value)}");
            }
        }
    }

    internal class RLSimulator
    {
        private RLDriver driver;

        public RLSimulator(LiveModel liveModel)
        {
            this.driver = new RLDriver(liveModel);
        }

        public TimeSpan StepInterval
        {
            get
            {
                return this.driver.StepInterval;
            }
            set
            {
                this.driver.StepInterval = value;
            }
        }

        public void Run(int steps = SimulatorStepProvider.InfinitySteps)
        {
            SimulatorStepProvider stepProvider = new SimulatorStepProvider(steps);

            this.driver.Run(stepProvider);
        }

        public event EventHandler<ApiStatus> OnError
        {
            add
            {
                this.driver.OnError += value;
            }
            remove
            {
                this.driver.OnError -= value;
            }
        }
    }
}
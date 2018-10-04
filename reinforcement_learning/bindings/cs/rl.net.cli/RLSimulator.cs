using System;
using System.IO;
using Rl.Net;

namespace Rl.Net.Cli {
    public enum Topic : long
    {
        HerbGarden,
        MachineLearning
    }

    internal class RLSimulator
    {
        public static readonly Random RandomSource = new Random();

        private LiveModel liveModel;
        private RankingResponse responseContainer;
        private ApiStatus apiStatusContainer;

        public RLSimulator(LiveModel liveModel)
        {
            this.liveModel = liveModel;
        }

        public void Run(int steps = -1)
        {
            this.responseContainer = new RankingResponse();
            this.apiStatusContainer = new ApiStatus();

            int stepsSoFar;
            while (steps < 0 || (stepsSoFar++ < steps))
            {
                this.Step();
            }
        }

        public event EventHandler<ApiStatus> OnError;

        public void Step()
        {
            Person person = GetRandomPerson();
            string decisionContext = CreateDecisionContext(person);

            Guid eventId = Guid.NewGuid();
            if (!liveModel.TryChooseRank(eventId.ToString(), decisionContext, this.responseContainer, this.apiStatusContainer))
            {
                this.SafeRaiseError(this.apiStatusContainer);
            }

            long actionId = -1;
            if (!responseContainer.TryGetChosenAction(out actionId, apiStatus))
            {
                this.SafeRaiseError(this.apiStatusContainer);
            }

            string actionTopic = ((Topic)actionId).ToString();
            float outcome = person.GenerateOutcome(actionTopic);

            if (!liveModel.TryReportOutcome(eventId.ToString(), outcome, this.apiStatusContainer))
            {
                this.SafeRaiseError(this.apiStatusContainer);
            }

            // TODO: Record stats

        }

        private void SafeRaiseError(ApiStatus errorStatus)
        {
            EventHandler<ApiStatus> localHandler = this.OnError;
            if (localHandler != null)
            {
                localHandler(errorStatus);
            }
        }

        private Func<Topic, float> GenerateRewardDistribution(float herbGardenProbability, float machineLearningProbability)
        {
            Dictionary<Topic, float> topicProbabilities = new Dictionary<Topic, float> 
            { 
                { Topic.HerbGarden.ToString(), 0.03f }, 
                { Topic.MachineLearning.ToString(), 0.1f }
            };

            return (topic) => topicProbabilities[topic];
        }

        private static Person[] People = new []
        {
            new Person("rnc", "engineering", "hiking", "spock", GenerateRewardDistribution(0.03f, 0.1f)),
            new Person("mk", "psychology", "kids", "7of9", GenerateRewardDistribution(0.3f, 0.1f))
        };

        private static readonly Topic[] ActionSet = new [] { Topic.HerbGarden, Topic.MachineLearning };

        private static readonly string ActionsJson = String.Join(",", ActionSet.Select(topic => $"{{ 'TAction': {{ 'topic': '{topic}' }} }}"));

        private string CreateDecisionContext(Person p, params Topic[] topics)
        {
            return $"{{ {p.FeaturesJson}, { ActionsJson } }}";
        }

        private Person GetRandomPerson()
        {
            int index = RandomSource.Next(People.Length);

            return People[index];
        }
    }
}
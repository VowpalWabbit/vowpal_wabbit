using System;
using System.IO;
using Rl.Net;

namespace Rl.Net.Cli {
    // TODO: We should really turn this into a separate library so that it can be consumed in tests.
    // In the short term, once we have a good setup, we should switch from internal to public.
    internal class Person
    {
        public Person(string id, string major, string hobby, string favoriteCharacter, Func<Topic, float> rewardProbabilityDistribution, Func<float> randomSource)
        {
            this.Id = id;
            this.Major = major;
            this.Hobby = hobby;
            this.FavoriteCharacter = favoriteCharacter;
            this.RewardProbabilityDistribution = rewardProbabilityDistribution;
            this.RandomSource = randomSource;
        }

        public Person(string id, string major, string hobby, string favoriteCharacter, Func<Topic, float> rewardProbabilityDistribution)
            : this(id, major, hobby, favoriteCharacter, rewardProbabilityDistribution, () => (float)RLSimulator.RandomSource.NextDouble())
        {}

        public string Id
        {
            get;
            private set;
        }

        public string Major
        {
            get;
            private set;
        }

        public string Hobby
        {
            get;
            private set;
        }

        public string FavoriteCharacter
        {
            get;
            private set;
        }

        public Func<Topic, float> RewardProbabilityDistribution
        {
            get;
            private set;
        }

        // TODO: Pluggable entropy source?
        private Func<float> RandomSource
        {
            get;
            set;
        }

        public string FeaturesJson
        {
            get
            {
                return $"'GUser':{{'id':'{this.Id}', 'major': '{this.Major}', 'hobby': '{this.Hobby}', 'favorite_character':'{this.FavoriteCharacter}'}}";
            }
        }

        public float GenerateOutcome(Topic topic)
        {
            float sample = this.RandomSource();
            float target = this.RewardProbabilityDistribution(topic);

            if (sample <= target)
            {
                return 1.0f;
            }
            else
            {
                return 0.0f;
            }
        }
    }
}
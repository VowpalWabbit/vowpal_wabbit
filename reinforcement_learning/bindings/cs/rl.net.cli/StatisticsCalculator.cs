using System;
using System.Collections.Generic;
using System.IO;
using Rl.Net;

namespace Rl.Net.Cli {
    internal class PersonStats
    {
        public int[,] ActionCounts
        {
            get;
            private set;
        } = new int[SimulatorStepProvider.SimulatorStep.ActionSet.Length, 2];

        public int TotalActions
        {
            get;
            private set;
        } = 0;

        public void IncrementAction(Topic action, float outcome)
        {
            if (outcome > 0.00001f)
            {
                this.ActionCounts[(int)action, 0]++;
            }

            this.ActionCounts[(int)action, 1]++;
            this.TotalActions++;
        }

        public string GetSummary(Topic action)
        {
            int wins = this.ActionCounts[(int)action, 0];
            int total = this.ActionCounts[(int)action, 1];

            return $"{action}: Out of {total} plays, {wins} wins. Person saw {this.TotalActions} decisions.";
        }
    }

    internal class StatisticsCalculator
    {
        private Dictionary<Person, PersonStats> personToStatsMap;

        public int TotalActions
        {
            get;
            private set;
        } = 0;

        public StatisticsCalculator()
        {
            this.personToStatsMap = new Dictionary<Person, PersonStats>();
        }

        private PersonStats EnsurePersonStats(Person person)
        {
            PersonStats result;
            if (!this.personToStatsMap.TryGetValue(person, out result))
            {
                this.personToStatsMap[person] = result = new PersonStats();
            }

            return result;
        }

        public void Record(Person person, Topic chosenAction, float outcome)
        {
            this.EnsurePersonStats(person).IncrementAction(chosenAction, outcome);
        }


        public string GetStats(Person person, Topic chosenAction)
        {
            return this.EnsurePersonStats(person).GetSummary(chosenAction);
        }
    }
}
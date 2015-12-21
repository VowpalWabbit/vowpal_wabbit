using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;

namespace MultiWorldTesting.MultiAction
{
    internal static class MultiActionHelper
    {
        internal static void ValidateActionList(uint[] actions)
        {
            bool[] exists = new bool[actions.Length + 1]; // plus 1 since action index is 1-based

            for (int i = 0; i < actions.Length; i++)
            {
                if (actions[i] == 0 || actions[i] > actions.Length)
                {
                    throw new ArgumentException("Action chosen by default policy is not within valid range.");
                }
                if (exists[actions[i]])
                {
                    throw new ArgumentException("List of actions cannot contain duplicates.");
                }
                exists[actions[i]] = true;
            }
        }

        internal static void PutActionToList(uint action, uint[] actionList)
        {
            for (int i = 0; i < actionList.Length; i++)
            {
                if (action == actionList[i])
                {
                    // swap;
                    actionList[i] = actionList[0];
                    actionList[0] = action;

                    return;
                }
            }
        }

        internal static uint[] SampleWithoutReplacement(List<float> probabilities, uint size, PRG randomGenerator, ref float topActionProbability)
        {
            for (int i = 0; i < size; i++)
            {
                if (probabilities[i] == 1f)
                {
                    throw new ArgumentException("The resulting probability distribution is deterministic and thus cannot generate a list of unique actions.");
                }
            }

            uint[] actions = Enumerable.Repeat<uint>(0, (int)size).ToArray();
            bool[] exists = new bool[actions.Length + 1]; // plus 1 since action index is 1-based

            // sample without replacement
            uint runningIndex = 0;
            uint runningAction = 0;
            float draw, sum;
            while (runningIndex < size)
            {
                draw = randomGenerator.UniformUnitInterval();
                sum = 0;

                for (int i = 0; i < size; i++)
                {
                    sum += probabilities[i];
                    if (sum > draw)
                    {
                        runningAction = (uint)(i + 1);

                        // check for duplicate
                        if (exists[runningAction])
                        {
                            continue;
                        }

                        // store newly sampled action
                        if (runningIndex == 0)
                        {
                            topActionProbability = probabilities[i];
                        }
                        actions[runningIndex++] = runningAction;
                        exists[runningAction] = true;
                        break;
                    }
                }
            }
            return actions;
        }
    }
}

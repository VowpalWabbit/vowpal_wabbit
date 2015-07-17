using System;
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
    }
}

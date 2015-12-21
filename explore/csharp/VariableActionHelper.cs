using System;
using System.Reflection;

namespace MultiWorldTesting
{
    internal static class VariableActionHelper
    {
        internal static void ValidateContextType<TContext>()
        {
            if (!typeof(IVariableActionContext).GetTypeInfo().IsAssignableFrom(typeof(TContext).GetTypeInfo()))
            {
                throw new ArgumentException("The generic context type does not implement IVariableActionContext interface.");
            }
        }

        internal static void ValidateNumberOfActions(uint numActions)
        {
            if (numActions != uint.MaxValue && numActions < 1)
            {
                throw new ArgumentException("Number of actions must be at least 1.");
            }
        }

        internal static uint GetNumberOfActions<TContext>(TContext context, uint defaultNumActions)
        { 
            uint numActions = defaultNumActions;
            if (numActions == uint.MaxValue)
            {
                numActions = ((IVariableActionContext)(context)).GetNumberOfActions();
                if (numActions < 1)
                {
                    throw new ArgumentException("Number of actions must be at least 1.");
                }
            }
            return numActions;
        }
    }
}

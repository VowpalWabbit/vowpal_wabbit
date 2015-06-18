using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MultiWorldTesting
{
    internal static class VariableActionHelper
    {
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

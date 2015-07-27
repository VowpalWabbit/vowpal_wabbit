using System;
using System.Linq;

namespace MultiWorldTesting.SingleAction
{
    /// <summary>
	/// The bootstrap exploration class.
	/// </summary>
	/// <remarks>
	/// The Bootstrap explorer randomizes over the actions chosen by a set of
	/// default policies.  This performs well statistically but can be
	/// computationally expensive.
	/// </remarks>
	/// <typeparam name="TContext">The Context type.</typeparam>
	public class BootstrapExplorer<TContext> : IExplorer<TContext>, IConsumePolicies<TContext>
	{
        private IPolicy<TContext>[] defaultPolicyFunctions;
        private bool explore;
        private readonly uint bags;
	    private readonly uint numActions;

		/// <summary>
		/// The constructor is the only public member, because this should be used with the MwtExplorer.
		/// </summary>
		/// <param name="defaultPolicies">A set of default policies to be uniform random over.</param>
		/// <param name="numActions">The number of actions to randomize over.</param>
        public BootstrapExplorer(IPolicy<TContext>[] defaultPolicies, uint numActions)
		{
            VariableActionHelper.ValidateNumberOfActions(numActions);

            if (defaultPolicies == null || defaultPolicies.Length < 1)
		    {
			    throw new ArgumentException("Number of bags must be at least 1.");
		    }

            this.defaultPolicyFunctions = defaultPolicies;
            this.bags = (uint)this.defaultPolicyFunctions.Length;
            this.numActions = numActions;
            this.explore = true;
        }

        /// <summary>
        /// Initializes a bootstrap explorer with variable number of actions.
        /// </summary>
        /// <param name="defaultPolicies">A set of default policies to be uniform random over.</param>
        public BootstrapExplorer(IPolicy<TContext>[] defaultPolicies) :
            this(defaultPolicies, uint.MaxValue)
        {
            VariableActionHelper.ValidateContextType<TContext>();
        }

        public void UpdatePolicy(IPolicy<TContext>[] newPolicies)
        {
            this.defaultPolicyFunctions = newPolicies;
        }

        public void EnableExplore(bool explore)
        {
            this.explore = explore;
        }

        public DecisionTuple Choose_Action(ulong saltedSeed, TContext context)
        {
            uint numActions = VariableActionHelper.GetNumberOfActions(context, this.numActions);

            var random = new PRG(saltedSeed);

            // Select bag
            uint chosenBag = random.UniformInt(0, this.bags - 1);

            // Invoke the default policy function to get the action
            uint chosenAction = 0;
            float actionProbability = 0f;

            if (this.explore)
            {
                uint actionFromBag = 0;
                uint[] actionsSelected = Enumerable.Repeat<uint>(0, (int)numActions).ToArray();

                // Invoke the default policy function to get the action
                for (int currentBag = 0; currentBag < this.bags; currentBag++)
                {
                    // TODO: can VW predict for all bags on one call? (returning all actions at once)
                    // if we trigger into VW passing an index to invoke bootstrap scoring, and if VW model changes while we are doing so, 
                    // we could end up calling the wrong bag
                    actionFromBag = this.defaultPolicyFunctions[currentBag].ChooseAction(context);

                    if (actionFromBag == 0 || actionFromBag > numActions)
                    {
                        throw new ArgumentException("Action chosen by default policy is not within valid range.");
                    }

                    if (currentBag == chosenBag)
                    {
                        chosenAction = actionFromBag;
                    }
                    //this won't work if actions aren't 0 to Count
                    actionsSelected[actionFromBag - 1]++; // action id is one-based
                }
                actionProbability = (float)actionsSelected[chosenAction - 1] / this.bags; // action id is one-based
            }
            else
            {
                chosenAction = this.defaultPolicyFunctions[0].ChooseAction(context);
                actionProbability = 1f;
            }
            return new DecisionTuple
            {
                Action = chosenAction,
                Probability = actionProbability,
                ShouldRecord = true
            };
        }
    };
}

namespace MultiWorldTesting.MultiAction
{
    /// <summary>
    /// The bootstrap exploration class.
    /// </summary>
    /// <remarks>
    /// The Bootstrap explorer randomizes over the actions chosen by a set of
    /// default policies.  This performs well statistically but can be
    /// computationally expensive.
    /// </remarks>
    /// <typeparam name="TContext">The Context type.</typeparam>
    public class BootstrapExplorer<TContext> : IExplorer<TContext>, IConsumePolicies<TContext>
    {
        private IPolicy<TContext>[] defaultPolicyFunctions;
        private bool explore;
        private readonly uint bags;
        private readonly uint numActions;

        /// <summary>
        /// The constructor is the only public member, because this should be used with the MwtExplorer.
        /// </summary>
        /// <param name="defaultPolicies">A set of default policies to be uniform random over.</param>
        /// <param name="numActions">The number of actions to randomize over.</param>
        public BootstrapExplorer(IPolicy<TContext>[] defaultPolicies, uint numActions)
        {
            VariableActionHelper.ValidateNumberOfActions(numActions);

            if (defaultPolicies == null || defaultPolicies.Length < 1)
            {
                throw new ArgumentException("Number of bags must be at least 1.");
            }

            this.defaultPolicyFunctions = defaultPolicies;
            this.bags = (uint)this.defaultPolicyFunctions.Length;
            this.numActions = numActions;
            this.explore = true;
        }

        /// <summary>
        /// Initializes a bootstrap explorer with variable number of actions.
        /// </summary>
        /// <param name="defaultPolicies">A set of default policies to be uniform random over.</param>
        public BootstrapExplorer(IPolicy<TContext>[] defaultPolicies) :
            this(defaultPolicies, uint.MaxValue)
        {
            VariableActionHelper.ValidateContextType<TContext>();
        }

        public void UpdatePolicy(IPolicy<TContext>[] newPolicies)
        {
            this.defaultPolicyFunctions = newPolicies;
        }

        public void EnableExplore(bool explore)
        {
            this.explore = explore;
        }

        public DecisionTuple ChooseAction(ulong saltedSeed, TContext context)
        {
            uint numActions = VariableActionHelper.GetNumberOfActions(context, this.numActions);

            var random = new PRG(saltedSeed);

            // Select bag
            uint chosenBag = random.UniformInt(0, this.bags - 1);

            // Invoke the default policy function to get the action
            uint[] chosenActions = null;
            uint chosenTopAction = 0;
            float actionProbability = 0f;

            if (this.explore)
            {
                uint topActionFromBag = 0;
                uint[] actionsFromBag = new uint[numActions];
                uint[] actionsSelected = Enumerable.Repeat<uint>(0, (int)numActions).ToArray();

                // Invoke the default policy function to get the action
                for (int currentBag = 0; currentBag < this.bags; currentBag++)
                {
                    // TODO: can VW predict for all bags on one call? (returning all actions at once)
                    // if we trigger into VW passing an index to invoke bootstrap scoring, and if VW model changes while we are doing so, 
                    // we could end up calling the wrong bag
                    actionsFromBag = this.defaultPolicyFunctions[currentBag].ChooseAction(context);

                    MultiActionHelper.ValidateActionList(actionsFromBag);

                    topActionFromBag = actionsFromBag[0];

                    if (currentBag == chosenBag)
                    {
                        chosenTopAction = topActionFromBag;
                        chosenActions = actionsFromBag;
                    }
                    //this won't work if actions aren't 0 to Count
                    actionsSelected[topActionFromBag - 1]++; // action id is one-based
                }
                actionProbability = (float)actionsSelected[chosenTopAction - 1] / this.bags; // action id is one-based
            }
            else
            {
                chosenActions = this.defaultPolicyFunctions[0].ChooseAction(context);
                actionProbability = 1f;
            }
            return new DecisionTuple
            {
                Actions = chosenActions,
                Probability = actionProbability,
                ShouldRecord = true
            };
        }
    };
}
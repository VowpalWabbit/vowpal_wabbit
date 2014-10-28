#pragma once

#include <float.h>
#include <functional>
#include <tuple>
#include <math.h>
#include "Interaction.h"

//
// Common interface for all exploration algorithms
//

/*  TODO: clean up these comments
	These classes are used internally within MwtExplorer.h. 
	Behavior of independent external usage is undefined. 
*/
MWT_NAMESPACE {

class Explorer
{
public:
	virtual std::tuple<MWTAction, float, bool> Choose_Action(void* context, ActionSet& actions, u32 seed) = 0;
	virtual ~Explorer() { }
};

class EpsilonGreedyExplorer : public Explorer
{
public:
	EpsilonGreedyExplorer(
		float epsilon,
		Stateful_Policy_Func* default_policy_func,
		void* default_policy_params,
		u64 salt) :
                m_epsilon(epsilon),
		m_salt(salt),
		m_stateful_default_policy_func(default_policy_func),
		m_stateless_default_policy_func(nullptr),
		m_default_policy_params(default_policy_params)
	{
	}

	EpsilonGreedyExplorer(
		float epsilon,
		Stateless_Policy_Func* default_policy_func,
		u64 salt) :
		m_epsilon(epsilon),
		m_salt(salt),
		m_stateful_default_policy_func(nullptr),
		m_stateless_default_policy_func(default_policy_func),
		m_default_policy_params(nullptr)
	{
	}

	~EpsilonGreedyExplorer()
	{
	}

	std::tuple<MWTAction, float, bool> Choose_Action(void* context, ActionSet& actions, u32 seed)
	{
		PRG::prg random_generator(m_salt + seed);
		// Invoke the default policy function to get the action
		MWTAction chosen_action(0);
		if (m_stateless_default_policy_func != nullptr)
		{
			chosen_action = MWTAction(m_stateless_default_policy_func(context));
		}
		else
		{
			chosen_action = MWTAction(m_stateful_default_policy_func(m_default_policy_params, context));
		}

		if (chosen_action.Get_Id() == 0 || chosen_action.Get_Id() > actions.Count())
		{
			throw std::invalid_argument("Action chosen by default policy is not within valid range.");
		}

		float action_probability = 0.f;
		float base_probability = m_epsilon / actions.Count(); // uniform probability

		// TODO: check this random generation
		if (random_generator.Uniform_Unit_Interval() < 1.f - m_epsilon)
		{
			action_probability = 1.f - m_epsilon + base_probability;
		}
		else
		{
			// Get uniform random action ID
			u32 actionId = random_generator.Uniform_Int(1, actions.Count());

			if (actionId == chosen_action.Get_Id())
			{
				// IF it matches the one chosen by the default policy
				// then increase the probability
				action_probability = 1.f - m_epsilon + base_probability;
			}
			else
			{
				// Otherwise it's just the uniform probability
				action_probability = base_probability;
			}
			chosen_action = actions.Get(actionId);
		}

		return std::tuple<MWTAction, float, bool>(chosen_action, action_probability, true);
	}

private:
	float m_epsilon;
	u64 m_salt;

	Stateful_Policy_Func* m_stateful_default_policy_func;
	Stateless_Policy_Func* m_stateless_default_policy_func;
	void* m_default_policy_params;
};


class SoftmaxExplorer : public Explorer
{
public:
	SoftmaxExplorer(
		float lambda,
		Stateful_Scorer_Func* default_scorer_func,
		void* default_scorer_params,
		u64 salt) :
		m_lambda(lambda),
		m_salt(salt),
		m_stateful_default_scorer_func(default_scorer_func),
		m_stateless_default_scorer_func(nullptr),
		m_default_scorer_params(default_scorer_params)
	{
	}

	SoftmaxExplorer(
		float lambda,
		Stateless_Scorer_Func* default_scorer_func,
		u64 salt) :
		m_lambda(lambda),
		m_salt(salt),
		m_stateful_default_scorer_func(nullptr),
		m_stateless_default_scorer_func(default_scorer_func),
		m_default_scorer_params(nullptr)
	{
	}

	std::tuple<MWTAction, float, bool> Choose_Action(void* context, ActionSet& actions, u32 seed)
	{
		PRG::prg random_generator(seed);
		MWTAction chosen_action(0);
		u32 numScores = actions.Count();
		std::unique_ptr<float[]> scores(new float[numScores]());
		// Invoke the default scorer function to score each action 
		if (m_stateless_default_scorer_func != nullptr)
		{
			m_stateless_default_scorer_func(context, scores.get(), actions.Count());
		}
		else
		{
			m_stateful_default_scorer_func(m_default_scorer_params, context, scores.get(), actions.Count());
		}

		u32 i = 0;

		float max_score = -FLT_MAX;
		for (i = 0; i < numScores; i++)
		{
			if (max_score < scores[i])
			{
				max_score = scores[i];
			}
		}

		// Create a normalized exponential distribution based on the returned scores
		for (i = 0; i < numScores; i++)
		{
			scores[i] = exp(m_lambda * (scores[i] - max_score));
		}

		// Create a discrete_distribution based on the returned weights. This class handles the
		// case where the sum of the weights is < or > 1, by normalizing agains the sum.
		float total = 0.f;
		for (size_t i = 0; i < numScores; i++)
		  total += scores[i];
		
		float draw = random_generator.Uniform_Unit_Interval();
		
		float sum = 0.f;
		float action_probability = 0.f;
		u32 action_index = numScores - 1;
		for (u32 i = 0; i < numScores; i++)
		{
			scores[i] = scores[i] / total;
			sum += scores[i];
			if (sum > draw)
			{
				action_index = i;
				action_probability = scores[i];
				break;
			}
		}

		return std::tuple<MWTAction, float, bool>(actions.Get(MWTAction::Make_OneBased(action_index)), action_probability, true);
 	}

private:
	float m_lambda;
	u64 m_salt;

	Stateful_Scorer_Func* m_stateful_default_scorer_func;
	Stateless_Scorer_Func* m_stateless_default_scorer_func;
	void* m_default_scorer_params;
};


class GenericExplorer : public Explorer
{
public:
	GenericExplorer(
		Stateful_Scorer_Func* default_scorer_func, 
		void* default_scorer_params,
		u64 salt) :
                m_salt(salt),
		m_stateful_default_scorer_func(default_scorer_func),
		m_stateless_default_scorer_func(nullptr),
		m_default_scorer_params(default_scorer_params)
	{
	}

	GenericExplorer(
		Stateless_Scorer_Func* default_scorer_func,
		u64 salt) :
		m_salt(salt),
		m_stateful_default_scorer_func(nullptr),
		m_stateless_default_scorer_func(default_scorer_func),
		m_default_scorer_params(nullptr)
	{
	}

	std::tuple<MWTAction, float, bool> Choose_Action(void* context, ActionSet& actions, u32 seed)
	{
		PRG::prg random_generator(seed);
		MWTAction chosen_action(0);
		u32 numWeights = actions.Count();
		std::unique_ptr<float[]> weights(new float[numWeights]());
		// Invoke the default scorer function to get the weight of each action 
		if (m_stateless_default_scorer_func != nullptr)
		{
			m_stateless_default_scorer_func(context, weights.get(), actions.Count());
		}
		else
		{
			m_stateful_default_scorer_func(m_default_scorer_params, context, weights.get(), actions.Count());
		}

		// Create a discrete_distribution based on the returned weights. This class handles the
		// case where the sum of the weights is < or > 1, by normalizing agains the sum.
		float total = 0.f;
		for (size_t i = 0; i < numWeights; i++)
		{
			if (weights[i] < 0)
			{
				throw std::invalid_argument("Scores must be non-negative.");
			}
			total += weights[i];
		}
		if (total == 0)
		{
			throw std::invalid_argument("At least one score must be positive.");
		}

		float draw = random_generator.Uniform_Unit_Interval();
		
		float sum = 0.f;
		float action_probability = 0.f;
		u32 action_index = numWeights-1;
		for (u32 i = 0; i < numWeights; i++)
		{
			weights[i] = weights[i] / total;
			sum += weights[i];
			if (sum > draw)
			{
				action_index = i;
				action_probability = weights[i];
				break;
			}
		}

		return std::tuple<MWTAction, float, bool>(actions.Get(MWTAction::Make_OneBased(action_index)), action_probability, true);
	}

private:
	u64 m_salt;

	Stateful_Scorer_Func* m_stateful_default_scorer_func;
	Stateless_Scorer_Func* m_stateless_default_scorer_func;
	void* m_default_scorer_params;
};


class TauFirstExplorer : public Explorer
{
public:
	TauFirstExplorer(
		u32 tau,
		Stateful_Policy_Func* default_policy_func,
		void* default_policy_params,
		u64 salt) :
		m_tau(tau),
		m_salt(salt),
		m_stateful_default_policy_func(default_policy_func),
		m_stateless_default_policy_func(nullptr),
		m_default_policy_params(default_policy_params)
	{
	}

	TauFirstExplorer(
		u32 tau,
		Stateless_Policy_Func* default_policy_func,
		u64 salt) :
		m_tau(tau),
		m_salt(salt),
		m_stateful_default_policy_func(nullptr),
		m_stateless_default_policy_func(default_policy_func),
		m_default_policy_params(nullptr)
	{
	}

	std::tuple<MWTAction, float, bool> Choose_Action(void* context, ActionSet& actions, u32 seed)
	{
		PRG::prg random_generator(seed + m_salt);
		MWTAction chosen_action(0);
		float action_probability = 0.f;
		bool log_action;
		if (m_tau)
		{
			m_tau--;
			u32 actionId = random_generator.Uniform_Int(1, actions.Count());
			action_probability = 1.f / actions.Count();
			chosen_action = actions.Get(actionId);
			log_action = true;
		}
		else
		{
			// Invoke the default policy function to get the action
			if (m_stateless_default_policy_func != nullptr)
			{
				chosen_action = MWTAction(m_stateless_default_policy_func(context));
			}
			else
			{
				chosen_action = MWTAction(m_stateful_default_policy_func(m_default_policy_params, context));
			}

			if (chosen_action.Get_Id() == 0 || chosen_action.Get_Id() > actions.Count())
			{
				throw std::invalid_argument("Action chosen by default policy is not within valid range.");
			}

			action_probability = 1.f;
			log_action = false;
		}

		return std::tuple<MWTAction, float, bool>(chosen_action, action_probability, log_action);
	}

private:
	u32 m_tau;
	u64 m_salt;

	Stateful_Policy_Func* m_stateful_default_policy_func;
	Stateless_Policy_Func* m_stateless_default_policy_func;
	void* m_default_policy_params;
};


class BaggingExplorer : public Explorer
{
public:
	BaggingExplorer(
		u32 bags,
		Stateful_Policy_Func** default_policy_functions,
		void** default_policy_args,
		u64 salt) :
		m_bags(bags),
		m_salt(salt),
		m_stateful_default_policy_funcs(default_policy_functions),
		m_stateless_default_policy_funcs(nullptr),
		m_default_policy_params(default_policy_args)
	{
	}

	BaggingExplorer(
		u32 bags,
		Stateless_Policy_Func** default_policy_functions,
		u64 salt) :
		m_bags(bags),
		m_salt(salt),
		m_stateful_default_policy_funcs(nullptr),
		m_stateless_default_policy_funcs(default_policy_functions),
		m_default_policy_params(nullptr)
	{
	}

	~BaggingExplorer()
	{
	}

	std::tuple<MWTAction, float, bool> Choose_Action(void* context, ActionSet& actions, u32 seed)
	{
		PRG::prg random_generator(seed + m_salt);
		// Select bag
		u32 chosen_bag = random_generator.Uniform_Int(0, m_bags - 1);
		// Invoke the default policy function to get the action
		MWTAction chosen_action(0);
		MWTAction action_from_bag(0);
		// Maybe be best to make this static size
		std::unique_ptr<u32[]> actions_selected(new u32[actions.Count()]);
		for (size_t i = 0; i < actions.Count(); i++)
		{
			actions_selected[i] = 0;
		}
		for (u32 current_bag = 0; current_bag < m_bags; current_bag++)
		{
			if (m_stateless_default_policy_funcs != nullptr)
			{
				action_from_bag = MWTAction(m_stateless_default_policy_funcs[current_bag](context));
			}
			else
			{
				action_from_bag = MWTAction(m_stateful_default_policy_funcs[current_bag](m_default_policy_params[current_bag], context));
			}

			if (action_from_bag.Get_Id() == 0 || action_from_bag.Get_Id() > actions.Count())
			{
				throw std::invalid_argument("Action chosen by default policy is not within valid range.");
			}

			if (current_bag == chosen_bag)
			{
				chosen_action = action_from_bag;
			}
			//this won't work if actions aren't 0 to Count
			actions_selected[action_from_bag.Get_Id_ZeroBased()]++;
		}
		float action_probability = (float)actions_selected[chosen_action.Get_Id_ZeroBased()] / m_bags;

		return std::tuple<MWTAction, float, bool>(chosen_action, action_probability, true);
	}

private:
	u32 m_bags;
	u64 m_salt;

	Stateful_Policy_Func** m_stateful_default_policy_funcs;
	Stateless_Policy_Func** m_stateless_default_policy_funcs;
	void** m_default_policy_params;
};
}

#pragma once

#include "Interaction.h"
#include <float.h>
//
// Common interface for all exploration algorithms
//
// TODO: for exploration budget, exploration algo should implement smth like Start & Stop Explore, Adjust epsilon
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
		void* default_policy_params) :
		m_epsilon(epsilon),
		m_stateful_default_policy_func(default_policy_func),
		m_stateless_default_policy_func(nullptr),
		m_default_policy_params(default_policy_params)
	{
	}

	EpsilonGreedyExplorer(
		float epsilon,
		Stateless_Policy_Func* default_policy_func) :
		m_epsilon(epsilon),
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
		PRG<u32> random_generator(seed);
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
		void* default_scorer_params) :
		m_lambda(lambda),
		m_stateful_default_scorer_func(default_scorer_func),
		m_stateless_default_scorer_func(nullptr),
		m_default_scorer_params(default_scorer_params)
	{
	}

	SoftmaxExplorer(
		float lambda,
		Stateless_Scorer_Func* default_scorer_func) :
		m_lambda(lambda),
		m_stateful_default_scorer_func(nullptr),
		m_stateless_default_scorer_func(default_scorer_func),
		m_default_scorer_params(nullptr)
	{
	}

	std::tuple<MWTAction, float, bool> Choose_Action(void* context, ActionSet& actions, u32 seed)
	{
		PRG<u32> random_generator(seed);
		MWTAction chosen_action(0);
		u32 numScores = actions.Count();
		float* scores = new float[numScores]();
		// Invoke the default scorer function to score each action 
		if (m_stateless_default_scorer_func != nullptr)
		{
			m_stateless_default_scorer_func(context, scores, actions.Count());
		}
		else
		{
			m_stateful_default_scorer_func(m_default_scorer_params, context, scores, actions.Count());
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
		//TODO: VS2013 doesn't support the iterator based constructor of discrete_distribution
		i = 0;
		std::discrete_distribution<u32> softmax_dist(numScores, 0, 1,  // 0 and 1 are nonsense parameters here
			[scores, &i](float)
		{
			auto w = scores[i];
			i++;
			return w;
		});
		// This retrives the PRG engine by reference, so evolving it will evolve the original PRG
		u32 action_index = softmax_dist(random_generator.Get_Engine());

		return std::tuple<MWTAction, float, bool>(actions.Get(MWTAction::Make_OneBased(action_index)), softmax_dist.probabilities()[action_index], true);
	}

private:
	float m_lambda;

	Stateful_Scorer_Func* m_stateful_default_scorer_func;
	Stateless_Scorer_Func* m_stateless_default_scorer_func;
	void* m_default_scorer_params;
};


template <class T>
class GenericExplorer : public Explorer
{
public:
	GenericExplorer(
		BaseFunctionWrapper& default_scorer_func_wrapper,
		T* default_scorer_params) :
		m_default_scorer_wrapper(default_scorer_func_wrapper),
		m_default_scorer_params(default_scorer_params)
	{
	}

	std::tuple<MWTAction, float, bool> Choose_Action(void* context, ActionSet& actions, u32 seed)
	{
		PRG<u32> random_generator(seed);
		MWTAction chosen_action(0);
		u32 numWeights = actions.Count();
		float* weights = new float[numWeights]();
		// Invoke the default scorer function to get the weight of each action 
		if (typeid(m_default_scorer_wrapper) == typeid(StatelessFunctionWrapper))
		{
			StatelessFunctionWrapper* stateless_function_wrapper = (StatelessFunctionWrapper*)(&m_default_scorer_wrapper);
			stateless_function_wrapper->m_scorer_function(context, weights, actions.Count());
		}
		else
		{
			StatefulFunctionWrapper<T>* stateful_function_wrapper = (StatefulFunctionWrapper<T>*)(&m_default_scorer_wrapper);
			stateful_function_wrapper->m_scorer_function(m_default_scorer_params, context, weights, actions.Count());
		}

		u32 i = 0;
		// Create a discrete_distribution based on the returned weights. This class handles the
		// case where the sum of the weights is < or > 1, by normalizing agains the sum.
		//TODO: VS2013 doesn't support the iterator based constructor of discrete_distribution
		std::discrete_distribution<u32> generic_dist(numWeights, 0, 1,  // 0 and 1 are nonsense parameters here
			[weights, &i](float)
		{
			auto w = weights[i];
			i++;
			return w;
		});
		// This retrives the PRG engine by reference, so evolving it will evolve the original PRG
		u32 action_index = generic_dist(random_generator.Get_Engine());

		return std::tuple<MWTAction, float, bool>(actions.Get(MWTAction::Make_OneBased(action_index)), generic_dist.probabilities()[action_index], true);
	}

private:
	BaseFunctionWrapper& m_default_scorer_wrapper;
	T* m_default_scorer_params;
};


class TauFirstExplorer : public Explorer
{
public:
	TauFirstExplorer(
		u32 tau,
		Stateful_Policy_Func* default_policy_func,
		void* default_policy_params) :
		m_tau(tau),
		m_stateful_default_policy_func(default_policy_func),
		m_stateless_default_policy_func(nullptr),
		m_default_policy_params(default_policy_params)
	{
	}

	TauFirstExplorer(
		u32 tau,
		Stateless_Policy_Func* default_policy_func) :
		m_tau(tau),
		m_stateful_default_policy_func(nullptr),
		m_stateless_default_policy_func(default_policy_func),
		m_default_policy_params(nullptr)
	{
	}

	std::tuple<MWTAction, float, bool> Choose_Action(void* context, ActionSet& actions, u32 seed)
	{
		PRG<u32> random_generator(seed);
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

			action_probability = 1.f;
			log_action = false;
		}

		return std::tuple<MWTAction, float, bool>(chosen_action, action_probability, log_action);
	}

private:
	u32 m_tau;

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
		void** default_policy_args) :
                m_bags(bags),
		m_stateful_default_policy_funcs(default_policy_functions),
		m_stateless_default_policy_funcs(nullptr),
		m_default_policy_params(default_policy_args)
	{
	}

	BaggingExplorer(
		u32 bags,
		Stateless_Policy_Func** default_policy_functions) :
                m_bags(bags),
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
		PRG<u32> random_generator(seed);
		// Select bag
		u32 chosen_bag = random_generator.Uniform_Int(0, m_bags - 1);
		// Invoke the default policy function to get the action
		MWTAction chosen_action(0);
		MWTAction action_from_bag(0);
		// Maybe be best to make this static size
		u32* actions_selected = new u32[actions.Count()];
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

			if (current_bag == chosen_bag)
			{
				chosen_action = action_from_bag;
			}
			//this won't work if actions aren't 0 to Count
			actions_selected[action_from_bag.Get_Id_ZeroBased()]++;
		}
		float action_probability = (float)actions_selected[chosen_action.Get_Id_ZeroBased()] / m_bags;
		delete actions_selected;

		return std::tuple<MWTAction, float, bool>(chosen_action, action_probability, true);
	}

private:
	u32 m_bags;

	Stateful_Policy_Func** m_stateful_default_policy_funcs;
	Stateless_Policy_Func** m_stateless_default_policy_funcs;
	void** m_default_policy_params;
};

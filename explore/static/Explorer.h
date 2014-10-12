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


// TODO: This template is probably not needed any more, since we are going to convert everything to void* internally
template <class T>
class EpsilonGreedyExplorer : public Explorer
{
public:
	EpsilonGreedyExplorer(
		float epsilon,
		BaseFunctionWrapper& default_policy_func_wrapper,
		T* default_policy_params) :
		m_epsilon(epsilon),
		m_default_policy_wrapper(default_policy_func_wrapper),
		m_default_policy_params(default_policy_params)
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
		if (typeid(m_default_policy_wrapper) == typeid(StatelessFunctionWrapper))
		{
			StatelessFunctionWrapper* stateless_function_wrapper = (StatelessFunctionWrapper*)(&m_default_policy_wrapper);
			chosen_action = MWTAction(stateless_function_wrapper->m_policy_function(context));
		}
		else
		{
			StatefulFunctionWrapper<T>* stateful_function_wrapper = (StatefulFunctionWrapper<T>*)(&m_default_policy_wrapper);
			chosen_action = MWTAction(stateful_function_wrapper->m_policy_function(m_default_policy_params, context));
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

	BaseFunctionWrapper& m_default_policy_wrapper;
	T* m_default_policy_params;
};


template <class T>
class SoftmaxExplorer : public Explorer
{
public:
	SoftmaxExplorer(
		float lambda,
		BaseFunctionWrapper& default_scorer_func_wrapper,
		T* default_scorer_params) :
		m_lambda(lambda),
		m_default_scorer_wrapper(default_scorer_func_wrapper),
		m_default_scorer_params(default_scorer_params)
	{
		if (lambda < 0)
		{
			throw std::invalid_argument("Lambda value must be non-negative.");
		}
	}

	std::tuple<MWTAction, float, bool> Choose_Action(void* context, ActionSet& actions, u32 seed)
	{
		PRG<u32> random_generator(seed);
		MWTAction chosen_action(0);
		u32 numScores = actions.Count();
		float* scores = new float[numScores]();
		// Invoke the default scorer function to score each action 
		if (typeid(m_default_scorer_wrapper) == typeid(StatelessFunctionWrapper))
		{
			StatelessFunctionWrapper* stateless_function_wrapper = (StatelessFunctionWrapper*)(&m_default_scorer_wrapper);
			stateless_function_wrapper->m_scorer_function(context, scores, actions.Count());
		}
		else
		{
			StatefulFunctionWrapper<T>* stateful_function_wrapper = (StatefulFunctionWrapper<T>*)(&m_default_scorer_wrapper);
			stateful_function_wrapper->m_scorer_function(m_default_scorer_params, context, scores, actions.Count());
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

	BaseFunctionWrapper& m_default_scorer_wrapper;
	T* m_default_scorer_params;
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


template <class T>
class TauFirstExplorer : public Explorer
{
public:
	TauFirstExplorer(
		u32 tau,
		BaseFunctionWrapper& default_policy_func_wrapper,
		T* default_policy_params) :
		m_tau(tau),
		m_default_policy_wrapper(default_policy_func_wrapper),
		m_default_policy_params(default_policy_params)
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
			if (typeid(m_default_policy_wrapper) == typeid(StatelessFunctionWrapper))
			{
				StatelessFunctionWrapper* stateless_function_wrapper = (StatelessFunctionWrapper*)(&m_default_policy_wrapper);
				chosen_action = MWTAction(stateless_function_wrapper->m_policy_function(context));
			}
			else
			{
				StatefulFunctionWrapper<T>* stateful_function_wrapper = (StatefulFunctionWrapper<T>*)(&m_default_policy_wrapper);
				chosen_action = MWTAction(stateful_function_wrapper->m_policy_function(m_default_policy_params, context));
			}

			action_probability = 1.f;
			log_action = false;
		}

		return std::tuple<MWTAction, float, bool>(chosen_action, action_probability, log_action);
	}

private:
	u32 m_tau;

	BaseFunctionWrapper& m_default_policy_wrapper;
	T* m_default_policy_params;
};


template <class T>
class BaggingExplorer : public Explorer
{
public:
	BaggingExplorer(
		u32 bags,
		Stateful_Policy_Func** default_policy_functions,
		void** default_policy_args) :
		m_bags(bags)
	{
		m_default_policy_funcs.clear();
		m_default_policy_params.clear();

		for (u32 i = 0; i < bags; i++){
			StatefulFunctionWrapper<void>* func_wrapper = new StatefulFunctionWrapper<void>();
			func_wrapper->m_policy_function = default_policy_functions[i];
			m_default_policy_funcs.push_back(func_wrapper);
			m_default_policy_params.push_back(default_policy_args[i]);
		}
	}

	BaggingExplorer(
		u32 bags,
		Stateless_Policy_Func** default_policy_functions) :
		m_bags(bags)
	{
		m_default_policy_funcs.clear();
		m_default_policy_params.clear();

		for (u32 i = 0; i < bags; i++){
			StatelessFunctionWrapper* func_wrapper = new StatelessFunctionWrapper();
			func_wrapper->m_policy_function = default_policy_functions[i];
			m_default_policy_funcs.push_back(func_wrapper);
		}
	}

	~BaggingExplorer()
	{
		for (size_t i = 0; i < m_default_policy_funcs.size(); i++)
		{
			delete m_default_policy_funcs[i];
		}

		m_default_policy_funcs.clear();
		m_default_policy_params.clear();
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
			if (typeid(*m_default_policy_funcs[current_bag]) == typeid(StatelessFunctionWrapper))
			{
				StatelessFunctionWrapper* stateless_function_wrapper = (StatelessFunctionWrapper*)(m_default_policy_funcs[current_bag]);
				action_from_bag = MWTAction(stateless_function_wrapper->m_policy_function(context));
			}
			else
			{
				StatefulFunctionWrapper<T>* stateful_function_wrapper = (StatefulFunctionWrapper<T>*)(m_default_policy_funcs[current_bag]);
				action_from_bag = MWTAction(stateful_function_wrapper->m_policy_function(m_default_policy_params[current_bag], context));
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

	std::vector<BaseFunctionWrapper*> m_default_policy_funcs;
	std::vector<T*> m_default_policy_params;
};

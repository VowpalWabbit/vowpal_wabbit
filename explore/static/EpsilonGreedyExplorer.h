#pragma once

#include "PolicyFunc.h"
#include "Explorer.h"

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
		m_random_generator = new PRG<u32>();
	}

	~EpsilonGreedyExplorer()
	{
		delete m_random_generator;
	}

	std::tuple<MWTAction, float, bool> Choose_Action(void* context, ActionSet& actions)
	{
		return this->Choose_Action(context, actions, *m_random_generator);
	}

	std::tuple<MWTAction, float, bool> Choose_Action(void* context, ActionSet& actions, u32 seed)
	{
		PRG<u32> random_generator(seed);
		return this->Choose_Action(context, actions, random_generator);
	}

private:
	std::tuple<MWTAction, float, bool> Choose_Action(void* context, ActionSet& actions, PRG<u32>& random_generator)
	{
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
	PRG<u32>* m_random_generator;

	BaseFunctionWrapper& m_default_policy_wrapper;
	T* m_default_policy_params;
};
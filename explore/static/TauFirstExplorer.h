#pragma once

#include "PolicyFunc.h"
#include "Explorer.h"

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
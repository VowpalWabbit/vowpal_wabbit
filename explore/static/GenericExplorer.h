#pragma once

#include "PolicyFunc.h"
#include "Explorer.h"

// TODO: This template is probably not needed any more, since we are going to convert everything to void* internally
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
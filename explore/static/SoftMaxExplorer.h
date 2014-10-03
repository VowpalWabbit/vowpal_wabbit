#pragma once

#include "PolicyFunc.h"
#include "Explorer.h"

// TODO: This template is probably not needed any more, since we are going to convert everything to void* internally
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
		m_random_generator = new PRG<u32>();
	}

	~SoftmaxExplorer()
	{
		delete m_random_generator;
	}

	std::tuple<MWTAction, float, bool> Choose_Action(Context& context, ActionSet& actions)
	{
		return this->Choose_Action(context, actions, *m_random_generator);
	}

	std::tuple<MWTAction, float, bool> Choose_Action(Context& context, ActionSet& actions, u32 seed)
	{
		PRG<u32> random_generator(seed);
		return this->Choose_Action(context, actions, random_generator);
	}

private:
	std::tuple<MWTAction, float, bool> Choose_Action(Context& context, ActionSet& actions, PRG<u32>& random_generator)
	{
		// Invoke the default scorer function to score each action 
		MWTAction chosen_action(0);
		u32 numScores = actions.Count();
		float* scores = new float[numScores]();
		if (typeid(m_default_scorer_wrapper) == typeid(StatelessFunctionWrapper))
		{
			StatelessFunctionWrapper* stateless_function_wrapper = (StatelessFunctionWrapper*)(&m_default_scorer_wrapper);
			stateless_function_wrapper->m_scorer_function(&context, scores, actions.Count());
		}
		else
		{
			StatefulFunctionWrapper<T>* stateful_function_wrapper = (StatefulFunctionWrapper<T>*)(&m_default_scorer_wrapper);
			stateful_function_wrapper->m_scorer_function(m_default_scorer_params, &context, scores, actions.Count());
		}

		u32 i = 0;
		// Create a normalized exponential distribution based on the returned scores
		for (i = 0; i < numScores; i++)
		{
			scores[i] = exp(m_lambda * scores[i]);
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

		return std::tuple<MWTAction, float, bool>(actions.Get(action_index), softmax_dist.probabilities()[action_index], true);
	}

private:
	float m_lambda;
	PRG<u32>* m_random_generator;

	BaseFunctionWrapper& m_default_scorer_wrapper;
	T* m_default_scorer_params;
};
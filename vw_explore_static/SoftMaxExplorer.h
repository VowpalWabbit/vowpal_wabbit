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
		T* default_scorer_func_state_context) :
		m_lambda(lambda),
		m_default_scorer_wrapper(default_scorer_func_wrapper),
		m_default_scorer_state_context(default_scorer_func_state_context)
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

	std::pair<MWTAction, float> Choose_Action(Context& context, ActionSet& actions)
	{
		return this->Choose_Action(context, actions, *m_random_generator);
	}

	std::pair<MWTAction, float> Choose_Action(Context& context, ActionSet& actions, u32 seed)
	{
		PRG<u32> random_generator(seed);
		return this->Choose_Action(context, actions, random_generator);
	}

private:
	std::pair<MWTAction, float> Choose_Action(Context& context, ActionSet& actions, PRG<u32>& random_generator)
	{
		// Invoke the default scorer function to score each action 
		MWTAction chosen_action(0);
		std::vector<float> scores;
		if (typeid(m_default_scorer_wrapper) == typeid(StatelessFunctionWrapper))
		{
			StatelessFunctionWrapper* stateless_function_wrapper = (StatelessFunctionWrapper*)(&m_default_scorer_wrapper);
			scores = stateless_function_wrapper->m_scorer_function(&context);
		}
		else
		{
			StatefulFunctionWrapper<T>* stateful_function_wrapper = (StatefulFunctionWrapper<T>*)(&m_default_scorer_wrapper);
			scores = stateful_function_wrapper->m_scorer_function(m_default_scorer_state_context, &context);
		}

		u32 i = 0;
		// Create a normalized exponential distribution based on the returned scores
		for (i = 0; i < scores.size(); i++)
		{
			scores[i] = exp(m_lambda * scores[i]);
		}
		i = 0;
		//TODO: VS2013 doesn't support the iterator based constructor of discrete_distribution
		std::discrete_distribution<u32> softmax_dist(scores.size(), 0, 1,  // 0 and 1 are nonsense parameters here
			[&scores, &i](float)
		{
			auto w = scores[i];
			++i;
			return w;
		});
		u32 action_index = softmax_dist(random_generator.Get_Engine());

		return std::pair<MWTAction, float>(actions.Get(action_index), softmax_dist.probabilities()[action_index]);
	}

private:
	float m_lambda;
	PRG<u32>* m_random_generator;

	BaseFunctionWrapper& m_default_scorer_wrapper;
	T* m_default_scorer_state_context;
};
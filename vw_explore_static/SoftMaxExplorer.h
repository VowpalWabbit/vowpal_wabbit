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
		// Invoke the default scorer function to get the score of each action 
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

		//TODO: THIS IS SILLY, IMPLEMENT DISCRETE DIST IN PRG INSTEAD
		double sum = 0.0;
		for (u32 i = 0; i < scores.size(); i++)
		{
			sum += exp(m_lambda * scores[i]);
		}
		double rand_unit = random_generator.Uniform_Unit();
		double cum = 0.0;
		float action_probability = 0.f;
		for (u32 i = 0; i < scores.size(); i++)
		{
			cum += exp(m_lambda * scores[i]) / sum;
			if (cum >= rand_unit)
			{
				chosen_action = actions.Get(i);
				action_probability = exp(m_lambda * scores[i]) / sum;
				break;
			}
		}

		return std::tuple<MWTAction, float, bool>(chosen_action, action_probability, true);
	}

private:
	float m_lambda;
	PRG<u32>* m_random_generator;

	BaseFunctionWrapper& m_default_scorer_wrapper;
	T* m_default_scorer_state_context;
};
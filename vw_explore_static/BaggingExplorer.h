template <class T>
class BaggingExplorer : public Explorer
{
public:
	BaggingExplorer(
		int bags,
		BaseFunctionWrapper* default_policy_func_wrapper_array,
		T* default_policy_func_state_context_array) :
		m_bags(bags),
		m_default_policy_wrapper_array(default_policy_func_wrapper_array),
		m_default_policy_state_context_array(default_policy_func_state_context_array)
	{
		if (bags <= 0)
		{
			throw std::invalid_argument("Initial bags value must be positive.");
		}
		m_random_generator = new PRG<u32>();
	}

	~BaggingExplorer()
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
		// Invoke the default policy function to get the action
		MWTAction* chosen_action = nullptr;
		if (typeid(m_default_policy_wrapper) == typeid(StatelessFunctionWrapper))
		{
			StatelessFunctionWrapper* stateless_function_wrapper = (StatelessFunctionWrapper*)(&m_default_policy_wrapper);
			chosen_action = &stateless_function_wrapper->m_policy_function(context);
		}
		else
		{
			StatefulFunctionWrapper<T>* stateful_function_wrapper = (StatefulFunctionWrapper<T>*)(&m_default_policy_wrapper);
			chosen_action = &stateful_function_wrapper->m_policy_function(m_default_policy_state_context, context);
		}

		float action_probability = 0.f;
		float base_probability = m_epsilon / actions.Count(); // uniform probability

		// TODO: check this random generation
		if (((double)random_generator.Uniform_Int() / (2e32 - 1)) < 1.f - m_epsilon)
		{
			action_probability = 1.f - m_epsilon + base_probability;
		}
		else
		{
			// Get uniform random action ID
			u32 actionId = random_generator.Uniform_Int(1, actions.Count());

			if (actionId == chosen_action->Get_Id())
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
			chosen_action = &actions.Get(actionId);
		}

		return std::pair<MWTAction, float>(*chosen_action, action_probability);
	}

private:
	int m_bags;
	PRG<u32>* m_random_generator;

	BaseFunctionWrapper* m_default_policy_wrapper_array;
	T* m_default_policy_state_context_array;
};
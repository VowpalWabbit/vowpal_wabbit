template <class T>
class TauFirstExplorer : public Explorer
{
public:
	TauFirstExplorer(
		int tau,
		BaseFunctionWrapper& default_policy_func_wrapper,
		T* default_policy_func_state_context) :
		m_tau(tau),
		m_default_policy_wrapper(default_policy_func_wrapper),
		m_default_policy_state_context(default_policy_func_state_context)
	{
		if (tau <= 0)
		{
			throw std::invalid_argument("Initial tau value must be positive.");
		}
		m_random_generator = new PRG<u32>();
	}

	~TauFirstExplorer()
	{
		delete m_random_generator;
	}

	std::pair<Action, float> Choose_Action(Context& context, ActionSet& actions)
	{
		return this->Choose_Action(context, actions, *m_random_generator);
	}

	std::pair<Action, float> Choose_Action(Context& context, ActionSet& actions, u32 seed)
	{
		PRG<u32> random_generator(seed);
		return this->Choose_Action(context, actions, random_generator);
	}

private:
	std::pair<Action, float> Choose_Action(Context& context, ActionSet& actions, PRG<u32>& random_generator)
	{

		Action* chosen_action = nullptr;
		float action_probability = 0.f;
		if (tau){
			tau--;
			u32 actionId = random_generator.Uniform_Int(1, actions.Count());
			action_probability = 1.f/actions.Count();
			chosen_action = &actions.Get(actionId);

		}
		else {

			// Invoke the default policy function to get the action

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

			action_probability = 1.f;
		}

		return std::pair<Action, float>(*chosen_action, action_probability);
	}

private:
	int m_tau;
	PRG<u32>* m_random_generator;

	BaseFunctionWrapper& m_default_policy_wrapper;
	T* m_default_policy_state_context;
};
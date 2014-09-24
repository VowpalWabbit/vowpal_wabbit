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
			throw std::invalid_argument("Bags value must be positive.");
		}
		m_random_generator = new PRG<u32>();
	}

	~BaggingExplorer()
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
	std::tuple<MWTAction, float,bool> Choose_Action(Context& context, ActionSet& actions, PRG<u32>& random_generator)
	{
		//Select Bag
		u32 chosen_bag = random_generator.Uniform_Int(1, m_bags);
		// Invoke the default policy function to get the action
		MWTAction* chosen_action = nullptr;
		MWTAction* action_from_bag = nullptr;
		u32 actions_selected[actions.Count()];
		for (int i = 0; i < actions.Count(); i++){
			actions_selected[i] = 0;
		}
		for (int current_bag = 0; current_bag < m_bags; current_bag++;){
			if (typeid(m_default_policy_wrapper) == typeid(StatelessFunctionWrapper))
			{
				StatelessFunctionWrapper* stateless_function_wrapper = (StatelessFunctionWrapper*)(m_default_policy_wrapper_array[current_bag]);
				action_from_bag = &stateless_function_wrapper->m_policy_function(context);
			}
			else
			{
				StatefulFunctionWrapper<T>* stateful_function_wrapper = (StatefulFunctionWrapper<T>*)(m_default_policy_wrapper_array[current_bag]);
				action_from_bag = &stateful_function_wrapper->m_policy_function(m_default_policy_state_context_array[current_bag], context);
			}
			if (current_bag == chosen_bag){
				chosen_action = action_from_bag;
			}
			//this won't work if actions aren't 0 to Count
			actions_selected[action_from_bag.get_ID()]++;
		}
		float action_probability = actions_selected[chosen_action.getID()]/actions.Count();

		return std::tuple<MWTAction, float, bool>(*chosen_action, action_probability, true);
	}

private:
	int m_bags;
	PRG<u32>* m_random_generator;

	BaseFunctionWrapper* m_default_policy_wrapper_array;
	T* m_default_policy_state_context_array;
};
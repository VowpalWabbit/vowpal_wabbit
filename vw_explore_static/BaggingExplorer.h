template <class T>
class BaggingExplorer : public Explorer
{
public:
	BaggingExplorer(
		u32 bags,
		std::vector<BaseFunctionWrapper*>* default_policy_func_wrapper_ptr_vec,
		std::vector<T*>* default_policy_func_state_context_ptr_vec) :
		m_bags(bags),
		m_default_policy_wrapper_ptr_vec(default_policy_func_wrapper_ptr_vec),
		m_default_policy_state_context_ptr_vec(default_policy_func_state_context_ptr_vec)
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
		MWTAction chosen_action(0);
		MWTAction action_from_bag(0);
		//maybe be best to make this static size
		u32* actions_selected = new u32[actions.Count()];
		for (int i = 0; i < actions.Count(); i++){
			actions_selected[i] = 0;
		}
		for (u32 current_bag = 0; current_bag < m_bags; current_bag++){
			if (typeid(m_default_policy_wrapper_ptr_vec[0]) == typeid(StatelessFunctionWrapper*))
			{
				StatelessFunctionWrapper* stateless_function_wrapper = (StatelessFunctionWrapper*)((*m_default_policy_wrapper_ptr_vec)[current_bag]);
				action_from_bag = MWTAction(stateless_function_wrapper->m_policy_function(&context));
			}
			else
			{
				StatefulFunctionWrapper<T>* stateful_function_wrapper = (StatefulFunctionWrapper<T>*)((*m_default_policy_wrapper_ptr_vec)[current_bag]);
				action_from_bag = MWTAction(stateful_function_wrapper->m_policy_function((*m_default_policy_state_context_ptr_vec)[current_bag], &context));
			}
			if (current_bag == chosen_bag){
				chosen_action = action_from_bag;
			}
			//this won't work if actions aren't 0 to Count
			actions_selected[action_from_bag.Get_Id()]++;
		}
		float action_probability = (float) actions_selected[chosen_action.Get_Id()]/ (float) actions.Count();
		delete actions_selected;


		return std::tuple<MWTAction, float, bool>(chosen_action, action_probability, true);
	}

private:
	u32 m_bags;
	PRG<u32>* m_random_generator;

	std::vector<BaseFunctionWrapper*>* m_default_policy_wrapper_ptr_vec;
	std::vector<T*>* m_default_policy_state_context_ptr_vec;
};
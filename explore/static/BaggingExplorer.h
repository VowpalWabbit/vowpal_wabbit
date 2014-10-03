template <class T>
class BaggingExplorer : public Explorer
{
public:
	BaggingExplorer(
		u32 bags,
		Stateful_Policy_Func** default_policy_functions,
		void** default_policy_args) :
		m_bags(bags)
	{
		m_default_policy_funcs.clear();
		m_default_policy_params.clear();

		for (u32 i = 0; i < bags; i++){
			StatefulFunctionWrapper<void>* func_wrapper = new StatefulFunctionWrapper<void>();
			func_wrapper->m_policy_function = default_policy_functions[i];
			m_default_policy_funcs.push_back(func_wrapper);
			m_default_policy_params.push_back(default_policy_args[i]);
		}
		m_random_generator = new PRG<u32>();
	}

	BaggingExplorer(
		u32 bags,
		Stateless_Policy_Func** default_policy_functions) :
		m_bags(bags)
	{
		m_default_policy_funcs.clear();
		m_default_policy_params.clear();

		for (u32 i = 0; i < bags; i++){
			StatelessFunctionWrapper* func_wrapper = new StatelessFunctionWrapper();
			func_wrapper->m_policy_function = default_policy_functions[i];
			m_default_policy_funcs.push_back(func_wrapper);
		}
		m_random_generator = new PRG<u32>();
	}

	~BaggingExplorer()
	{
		delete m_random_generator;

		for (size_t i = 0; i < m_default_policy_funcs.size(); i++)
		{
			delete m_default_policy_funcs[i];
		}

		m_default_policy_funcs.clear();
		m_default_policy_params.clear();
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
		//Select Bag
		u32 chosen_bag = random_generator.Uniform_Int(0, m_bags - 1);
		// Invoke the default policy function to get the action
		MWTAction chosen_action(0);
		MWTAction action_from_bag(0);
		//maybe be best to make this static size
		u32* actions_selected = new u32[actions.Count()];
		for (int i = 0; i < actions.Count(); i++)
		{
			actions_selected[i] = 0;
		}
		for (u32 current_bag = 0; current_bag < m_bags; current_bag++)
		{
			if (typeid(*m_default_policy_funcs[current_bag]) == typeid(StatelessFunctionWrapper))
			{
				StatelessFunctionWrapper* stateless_function_wrapper = (StatelessFunctionWrapper*)(m_default_policy_funcs[current_bag]);
				action_from_bag = MWTAction(stateless_function_wrapper->m_policy_function(context));
			}
			else
			{
				StatefulFunctionWrapper<T>* stateful_function_wrapper = (StatefulFunctionWrapper<T>*)(m_default_policy_funcs[current_bag]);
				action_from_bag = MWTAction(stateful_function_wrapper->m_policy_function(m_default_policy_params[current_bag], context));
			}

			if (current_bag == chosen_bag)
			{
				chosen_action = action_from_bag;
			}
			//this won't work if actions aren't 0 to Count
			actions_selected[action_from_bag.Get_Id_ZeroBased()]++;
		}
		float action_probability = (float)actions_selected[chosen_action.Get_Id_ZeroBased()] / m_bags;
		delete actions_selected;

		return std::tuple<MWTAction, float, bool>(chosen_action, action_probability, true);
	}

private:
	u32 m_bags;
	PRG<u32>* m_random_generator;

	std::vector<BaseFunctionWrapper*> m_default_policy_funcs;
	std::vector<T*> m_default_policy_params;
};

//
// Top-level internal API for offline evaluation/optimization.
//
class MWTOptimizer
{
public:
	MWTOptimizer(size_t& num_interactions, Interaction* interactions[], u32 num_actions) 
		: m_num_actions(num_actions)
	{
		//TODO: Accept an ActionSet param that we'll use to call Match()? Maybe we should just accept a 
		// Match() method

		for (u64 i = 0; i < num_interactions; i++)
		{
			// Datasets returned by MWT apis should not contain null entries, but we check here
			// in case the user modified/mishandled the dataset. 
			if (interactions[i])
			{
				m_interactions.push_back(interactions[i]);
			}
		}
		m_num_actions = num_actions;
	}

	template <class T>
	float Evaluate_Policy(
		typename StatefulFunctionWrapper<T>::Policy_Func policy_func,
		T* policy_params)
	{
	  return this->Evaluate_Policy((Stateful_Policy_Func*)policy_func, (void*)policy_params);
	}

	float Evaluate_Policy(StatelessFunctionWrapper::Policy_Func policy_func)
	{
	  return this->Evaluate_Policy((Stateless_Policy_Func*)policy_func);
	}

	float Evaluate_Policy_VW_CSOAA(std::string model_input_file)
	{
		VW_HANDLE vw;
		VW_EXAMPLE example;
		double sum_weighted_rewards = 0.0;
		u64 count = 0;

		std::string params = "-i " + model_input_file + " --noconstant --quiet";
		vw = VW_InitializeA(params.c_str());
		MWTAction policy_action(0);
		for (auto pInteraction : m_interactions)
		{
			std::ostringstream serialized_stream;
			pInteraction->Serialize_VW_CSOAA(serialized_stream);
			example = VW_ReadExampleA(vw, serialized_stream.str().c_str());
			policy_action = MWTAction((u32)VW_Predict(vw, example));
			// If the policy action matches the action logged in the interaction, include the
			// (importance-weighted) reward in our average
			MWTAction a =pInteraction->Get_Action();
			if (policy_action.Match(a))
			{
				sum_weighted_rewards += pInteraction->Get_Reward() * (1.0 / pInteraction->Get_Prob());
				count++;
			}
		}
		VW_Finish(vw);
		return (sum_weighted_rewards / count);
	}

	void Optimize_Policy_VW_CSOAA(std::string model_output_file)
	{
		VW_HANDLE vw;
		VW_EXAMPLE example;

		std::string params = "--csoaa " + std::to_string(m_num_actions) + "--noconstant --quiet -f " + model_output_file;

		vw = VW_InitializeA(params.c_str());
		for (auto pInteraction : m_interactions)
		{
			std::ostringstream serialized_stream;
			pInteraction->Serialize_VW_CSOAA(serialized_stream);
			example = VW_ReadExampleA(vw, serialized_stream.str().c_str());	
			(void)VW_Learn(vw, example);
		}
		VW_Finish(vw);
	}

public:
	float Evaluate_Policy(
		Stateful_Policy_Func policy_func,
		void* policy_params)
	{
		StatefulFunctionWrapper<void> func_Wrapper = StatefulFunctionWrapper<void>();
		func_Wrapper.m_policy_function = (Stateful_Policy_Func*)policy_func;

		return Evaluate_Policy<void>(func_Wrapper, policy_params);
	}

	float Evaluate_Policy(Stateless_Policy_Func policy_func)
	{
		StatelessFunctionWrapper func_Wrapper = StatelessFunctionWrapper();
		func_Wrapper.m_policy_function = (Stateless_Policy_Func*)policy_func;

		return Evaluate_Policy<MWT_Empty>(func_Wrapper, nullptr);
	}

private:
	template <class T>
	float Evaluate_Policy(
		BaseFunctionWrapper& policy_func_wrapper,
		T* policy_params)
	{
		double sum_weighted_rewards = 0.0;
		u64 count = 0;

		for (auto pInteraction : m_interactions)
		{
			MWTAction policy_action(0);
			if (typeid(policy_func_wrapper) == typeid(StatelessFunctionWrapper))
			{
				StatelessFunctionWrapper* stateless_function_wrapper = (StatelessFunctionWrapper*)(&policy_func_wrapper);
				policy_action = MWTAction(stateless_function_wrapper->m_policy_function(pInteraction->Get_Context()));
			}
			else
			{
				StatefulFunctionWrapper<T>* stateful_function_wrapper = (StatefulFunctionWrapper<T>*)(&policy_func_wrapper);
				policy_action = MWTAction(stateful_function_wrapper->m_policy_function(policy_params, pInteraction->Get_Context()));
			}
			// If the policy action matches the action logged in the interaction, include the
			// (importance-weighted) reward in our average
			MWTAction a = pInteraction->Get_Action();
			if (policy_action.Match(a))
			{
				sum_weighted_rewards += pInteraction->Get_Reward() * (1.0 / pInteraction->Get_Prob());
				count++;
			}
		}

		return (sum_weighted_rewards / count);
	}


private:
	std::vector<Interaction*> m_interactions;
	u32 m_num_actions;
};

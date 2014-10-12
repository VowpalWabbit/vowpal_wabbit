//
// Main interface for clients to the MWT service.
//

#pragma once

#include "stdafx.h"
#include <typeinfo>
#include "hash.h"
#include "Explorer.h"
#include "vwdll.h"

//
// Top-level internal API for exploration (randomized decision making).
//
class MWTExplorer
{
public:
	MWTExplorer()
	{
		IdGenerator::Initialize();

		m_explorer = nullptr;
		m_action_set = nullptr;
	}

	~MWTExplorer()
	{
		IdGenerator::Destroy();

		delete m_explorer;
		delete m_action_set;
	}

	template <class T>
	void Initialize_Epsilon_Greedy(
		float epsilon, 
		typename StatefulFunctionWrapper<T>::Policy_Func default_policy_func, 
		T* default_policy_params, u32 num_actions)
	{
		this->Initialize_Epsilon_Greedy(epsilon, (Stateful_Policy_Func*)default_policy_func, (void*)default_policy_params, num_actions);
	}

	void Initialize_Epsilon_Greedy(
		float epsilon, 
		StatelessFunctionWrapper::Policy_Func default_policy_func,
		u32 num_actions)
	{
		this->Initialize_Epsilon_Greedy(epsilon, (Stateless_Policy_Func*)default_policy_func, num_actions);
	}

	/* Tau-first initialization */
	template <class T>
	void Initialize_Tau_First(
		u32 tau, 
		typename StatefulFunctionWrapper<T>::Policy_Func default_policy_func, 
		T* default_policy_params,
		u32 num_actions)
	{
		this->Initialize_Tau_First(tau, (Stateful_Policy_Func*)default_policy_func, (void*)default_policy_params, num_actions);
	}

	void Initialize_Tau_First(
		u32 tau, 
		StatelessFunctionWrapper::Policy_Func default_policy_func,
		u32 num_actions)
	{
		this->Initialize_Tau_First(tau, (Stateless_Policy_Func*)default_policy_func, num_actions);
	}

	/* Bagging initialization */
	template <class T>
	void Initialize_Bagging(
		u32 bags,
		typename StatefulFunctionWrapper<T>::Policy_Func** default_policy_functions,
		T** default_policy_params,
		u32 num_actions)
	{
		this->Initialize_Bagging(bags, (Stateful_Policy_Func**)default_policy_functions, (void**)default_policy_params, num_actions);
	}

	void Initialize_Bagging(
		u32 bags,
		StatelessFunctionWrapper::Policy_Func** default_policy_functions,
		u32 num_actions)
	{
		this->Initialize_Bagging(bags, (Stateless_Policy_Func**)default_policy_functions, num_actions);
	}

	/* Softmax initialization */
	template <class T>
	void Initialize_Softmax(
		float lambda,
		typename StatefulFunctionWrapper<T>::Scorer_Func default_scorer_func,
		T* default_scorer_params, u32 num_actions)
	{
		this->Initialize_Softmax(lambda, (Stateful_Scorer_Func*)default_scorer_func, (void*)default_scorer_params, num_actions);
	}

	void Initialize_Softmax(
		float lambda,
		StatelessFunctionWrapper::Scorer_Func default_scorer_func,
		u32 num_actions)
	{
		this->Initialize_Softmax(lambda, (Stateless_Scorer_Func*)default_scorer_func, num_actions);
	}

	std::pair<u32, u64> Choose_Action_And_Key(Context& context)
	{
		return this->Choose_Action_And_Key(&context, context);
	}

	// TODO: check whether char* could be std::string
	u32 Choose_Action(Context& context, std::string unique_id)
	{
		return this->Choose_Action(&context, unique_id, context);
	}

// Cross-language interface
public:
	void Initialize_Epsilon_Greedy(
		float epsilon, 
		Stateful_Policy_Func default_policy_func, 
		void* default_policy_func_argument, 
		u32 num_actions)
	{
		m_action_set = new ActionSet(num_actions);

		StatefulFunctionWrapper<void>* func_Wrapper = new StatefulFunctionWrapper<void>();
		func_Wrapper->m_policy_function = default_policy_func;
		
		m_explorer = new EpsilonGreedyExplorer<void>(epsilon, *func_Wrapper, default_policy_func_argument);
	}

	void Initialize_Epsilon_Greedy(
		float epsilon, 
		Stateless_Policy_Func default_policy_func,
		u32 num_actions)
	{
		m_action_set = new ActionSet(num_actions);

		StatelessFunctionWrapper* func_Wrapper = new StatelessFunctionWrapper();
		func_Wrapper->m_policy_function = default_policy_func;
		
		m_explorer = new EpsilonGreedyExplorer<MWT_Empty>(epsilon, *func_Wrapper, nullptr);
	}

	void Initialize_Tau_First(
		u32 tau, 
		Stateful_Policy_Func default_policy_func, 
		void* default_policy_func_argument,
		u32 num_actions)
	{
		m_action_set = new ActionSet(num_actions);

		StatefulFunctionWrapper<void>* func_wrapper = new StatefulFunctionWrapper<void>();
		func_wrapper->m_policy_function = default_policy_func;
		
		m_explorer = new TauFirstExplorer<void>(tau, *func_wrapper, default_policy_func_argument);
	}

	void Initialize_Tau_First(
		u32 tau, 
		Stateless_Policy_Func default_policy_func,
		u32 num_actions)
	{
		m_action_set = new ActionSet(num_actions);

		StatelessFunctionWrapper* func_wrapper = new StatelessFunctionWrapper();
		func_wrapper->m_policy_function = default_policy_func;
		
		m_explorer = new TauFirstExplorer<MWT_Empty>(tau, *func_wrapper, nullptr);
	}

	void Initialize_Bagging(
		u32 bags,
		Stateful_Policy_Func** default_policy_functions,
		void** default_policy_args,
		u32 num_actions)
	{
		m_action_set = new ActionSet(num_actions);
		m_explorer = new BaggingExplorer<void>(bags, default_policy_functions, default_policy_args);
	}

	void Initialize_Bagging(
		u32 bags,
		Stateless_Policy_Func** default_policy_functions,
		u32 num_actions)
	{
		m_action_set = new ActionSet(num_actions);
		m_explorer = new BaggingExplorer<void>(bags, default_policy_functions);
	}

	void Initialize_Softmax(
		float lambda,
		Stateful_Scorer_Func default_scorer_func,
		void* default_scorer_func_argument,
		u32 num_actions)
	{
		m_action_set = new ActionSet(num_actions);

		StatefulFunctionWrapper<void>* func_Wrapper = new StatefulFunctionWrapper<void>();
		func_Wrapper->m_scorer_function = default_scorer_func;

		m_explorer = new SoftmaxExplorer<void>(lambda, *func_Wrapper, default_scorer_func_argument);
	}

	void Initialize_Softmax(
		float lambda,
		Stateless_Scorer_Func default_scorer_func,
		u32 num_actions)
	{
		m_action_set = new ActionSet(num_actions);

		StatelessFunctionWrapper* func_wrapper = new StatelessFunctionWrapper();
		func_wrapper->m_scorer_function = default_scorer_func;

		m_explorer = new SoftmaxExplorer<MWT_Empty>(lambda, *func_wrapper, nullptr);
	}

	u32 Choose_Action(void* context, feature* context_features, size_t num_features, std::string* other_context, std::string unique_id)
	{
		Context log_context(context_features, num_features, other_context);
		return this->Choose_Action(context, unique_id, log_context);
	}

	// The parameters here look weird but are required to interface with C#:
	// The void* and Context& parameters are references to the same Context object.
	// Void* is required to pass back to the default policy function which could live in either native or managed space.
	// Context& is used internally to log data only since we need to access its members for serialization.
	u32 Choose_Action(void* context, std::string unique_id, Context& log_context)
	{
		u32 seed = this->Compute_Seed(const_cast<char*>(unique_id.c_str()), unique_id.size());
		std::tuple<MWTAction, float, bool> action_Probability_Log_Tuple = m_explorer->Choose_Action(context, *m_action_set, seed);
		
		if (!std::get<2>(action_Probability_Log_Tuple))
		{
			return std::get<0>(action_Probability_Log_Tuple).Get_Id();
		}

		Interaction pInteraction(&log_context, std::get<0>(action_Probability_Log_Tuple), std::get<1>(action_Probability_Log_Tuple), seed);
		m_logger.Store(&pInteraction);

		return std::get<0>(action_Probability_Log_Tuple).Get_Id();
	}

	// The parameters here look weird but are required to interface with C#
	std::pair<u32, u64> Choose_Action_And_Key(void* context, Context& log_context)
	{
		// Generate an ID for this interaction and use this to seed the PRG within the explorer
		u64 id = IdGenerator::Get_Id();
		std::tuple<MWTAction, float, bool> action_Probability_Log_Tuple = m_explorer->Choose_Action(context, *m_action_set, (u32)id);
		if (!std::get<2>(action_Probability_Log_Tuple))
		{
			// Since we aren't logging the interaction, don't return a join key (we are effectively
			// throwing away this ID, but so be it)
			return std::pair<u32, u64>(std::get<0>(action_Probability_Log_Tuple).Get_Id(), NO_JOIN_KEY);
		}
		Interaction interaction(&log_context, std::get<0>(action_Probability_Log_Tuple), std::get<1>(action_Probability_Log_Tuple), id);
		m_logger.Store(&interaction);

		return std::pair<u32, u64>(std::get<0>(action_Probability_Log_Tuple).Get_Id(), interaction.Get_Id());
	}

	std::string Get_All_Interactions()
	{
		return m_logger.Get_All_Interactions();
	}

	void Get_All_Interactions(size_t& num_interactions, Interaction**& interactions)
	{
		m_logger.Get_All_Interactions(num_interactions, interactions);
	}

private:
	u32 Compute_Seed(char* unique_id, u32 length)
	{
		// TODO: change return type to u64, may need to revisit this hash function
		return ::uniform_hash(unique_id, length, 0);
	}

private:
	Explorer* m_explorer;
	Logger m_logger;
	ActionSet* m_action_set;
};


//
// Top-level internal API for joining reward information to interaction data.
//
class MWTRewardReporter
{
public:
	MWTRewardReporter(size_t& num_interactions, Interaction* interactions[])
	{
		for (u64 i = 0; i < num_interactions; i++)
		{
			// Datasets returned by MWT apis should not contain null entries, but we check here
			// in case the user modified/mishandled the dataset. 
			if (interactions[i])
			{
				m_interactions[interactions[i]->Get_Id()] = interactions[i];
			}
		}
	}

	bool ReportReward(u64 id, float reward)
	{
		bool id_present = false;
		if (m_interactions.find(id) != m_interactions.end())
		{
			id_present = true;
			m_interactions[id]->Set_Reward(reward);
		}
		return id_present;			
	}

	bool ReportReward(size_t num_entries, u64 ids[], float rewards[])
	{
		bool all_ids_present = false;
		for (u64 i = 0; i < num_entries; i++)
		{
			all_ids_present &= ReportReward(ids[i], rewards[i]);
		}
		return all_ids_present;
	}

	std::string Get_All_Interactions()
	{
		std::ostringstream serialized_stream;
		for (auto interaction : m_interactions)
		{
			interaction.second->Serialize(serialized_stream);
		}
		return serialized_stream.str();
	}

	//TODO: Add interface to get all interactions as array? How about get all complete interactions? 
	// Or something to set the reward for all incomplete interactions?

private:
	std::map<u64, Interaction*> m_interactions;
};


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
		this->Evaluate_Policy((Stateful_Policy_Func*)policy_func, (void*)policy_params);
	}

	float Evaluate_Policy(StatelessFunctionWrapper::Policy_Func policy_func)
	{
		this->Evaluate_Policy((Stateless_Policy_Func*)policy_func);
	}

	float Evaluate_Policy_VW_CSOAA(std::string model_input_file)
	{
		VW_HANDLE vw;
		VW_EXAMPLE example;
		u32 action;
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
			if (policy_action.Match(pInteraction->Get_Action()))
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
			if (policy_action.Match(pInteraction->Get_Action()))
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

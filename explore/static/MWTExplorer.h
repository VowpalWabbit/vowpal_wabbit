//
// Main interface for clients to the MWT service.
//

#pragma once

#include "Common.h"
#include "Explorer.h"
#include <tuple>

//
// Top-level internal API for exploration (randomized decision making).
//
class MWTExplorer
{
public:
	MWTExplorer(std::string app_id)
	{
		m_explorer = nullptr;
		m_app_id = HashUtils::Compute_Id_Hash(app_id);
	}

	~MWTExplorer()
	{
		delete m_explorer;
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

	/* Generic initialization */
	template <class T>
	void Initialize_Generic(
		typename StatefulFunctionWrapper<T>::Scorer_Func default_scorer_func,
		T* default_scorer_params, u32 num_actions)
	{
		this->Initialize_Generic((Stateful_Scorer_Func*)default_scorer_func, (void*)default_scorer_params, num_actions);
	}

	void Initialize_Generic(
		StatelessFunctionWrapper::Scorer_Func default_scorer_func, u32 num_actions)
	{
		this->Initialize_Generic((Stateless_Scorer_Func*)default_scorer_func, num_actions);
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
		m_action_set.Set_Count(num_actions);
		m_explorer = new EpsilonGreedyExplorer(epsilon, default_policy_func, default_policy_func_argument, m_app_id);
	}

	void Initialize_Epsilon_Greedy(
		float epsilon, 
		Stateless_Policy_Func default_policy_func,
		u32 num_actions)
	{
		m_action_set.Set_Count(num_actions);
		m_explorer = new EpsilonGreedyExplorer(epsilon, default_policy_func, m_app_id);
	}

	void Initialize_Tau_First(
		u32 tau, 
		Stateful_Policy_Func default_policy_func, 
		void* default_policy_func_argument,
		u32 num_actions)
	{
		m_action_set.Set_Count(num_actions);
		m_explorer = new TauFirstExplorer(tau, default_policy_func, default_policy_func_argument, m_app_id);
	}

	void Initialize_Tau_First(
		u32 tau, 
		Stateless_Policy_Func default_policy_func,
		u32 num_actions)
	{
		m_action_set.Set_Count(num_actions);
		m_explorer = new TauFirstExplorer(tau, default_policy_func, m_app_id);
	}

	void Initialize_Bagging(
		u32 bags,
		Stateful_Policy_Func** default_policy_functions,
		void** default_policy_args,
		u32 num_actions)
	{
		m_action_set.Set_Count(num_actions);
		m_explorer = new BaggingExplorer(bags, default_policy_functions, default_policy_args, m_app_id);
	}

	void Initialize_Bagging(
		u32 bags,
		Stateless_Policy_Func** default_policy_functions,
		u32 num_actions)
	{
		m_action_set.Set_Count(num_actions);
		m_explorer = new BaggingExplorer(bags, default_policy_functions, m_app_id);
	}

	void Initialize_Softmax(
		float lambda,
		Stateful_Scorer_Func default_scorer_func,
		void* default_scorer_func_argument,
		u32 num_actions)
	{
		m_action_set.Set_Count(num_actions);
		m_explorer = new SoftmaxExplorer(lambda, default_scorer_func, default_scorer_func_argument, m_app_id);
	}

	void Initialize_Softmax(
		float lambda,
		Stateless_Scorer_Func default_scorer_func,
		u32 num_actions)
	{
		m_action_set.Set_Count(num_actions);
		m_explorer = new SoftmaxExplorer(lambda, default_scorer_func, m_app_id);
	}

	void Initialize_Generic(
		Stateful_Scorer_Func default_scorer_func,
		void* default_scorer_func_argument,
		u32 num_actions)
	{
		m_action_set.Set_Count(num_actions);
		m_explorer = new GenericExplorer(default_scorer_func, default_scorer_func_argument, m_app_id);
	}

	void Initialize_Generic(
		Stateless_Scorer_Func default_scorer_func,
		u32 num_actions)
	{
		m_action_set.Set_Count(num_actions);
		m_explorer = new GenericExplorer(default_scorer_func, m_app_id);
	}

	// The parameters here look weird but are required to interface with C#:
	// The void* and Context& parameters are references to the same Context object.
	// Void* is required to pass back to the default policy function which could live in either native or managed space.
	// Context& is used internally to log data only since we need to access its members for serialization.
	u32 Choose_Action(void* context, std::string unique_id, Context& log_context)
	{
		// Hash the ID of the yet-to-be-created interaction so we can seed the explorer
		u64 seed = HashUtils::Compute_Id_Hash(unique_id);
		if (m_explorer == nullptr)
		  throw std::invalid_argument("Error: you must initialize an explorer before use");
		std::tuple<MWTAction, float, bool> action_Probability_Log_Tuple = m_explorer->Choose_Action(context, m_action_set, seed);
		
		if (!std::get<2>(action_Probability_Log_Tuple))
		{
			return std::get<0>(action_Probability_Log_Tuple).Get_Id();
		}
		// Create an interaction using the same unique_id as used in the seed above!
		Interaction pInteraction(&log_context, std::get<0>(action_Probability_Log_Tuple), std::get<1>(action_Probability_Log_Tuple), unique_id);
		m_logger.Store(&pInteraction);

		return std::get<0>(action_Probability_Log_Tuple).Get_Id();
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
	Explorer* m_explorer;
	Logger m_logger;
	ActionSet m_action_set;
	u64 m_app_id;
};

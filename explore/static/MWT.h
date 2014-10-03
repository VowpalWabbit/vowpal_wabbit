//
// Main interface for clients to the MWT service.
//

#pragma once

#include "stdafx.h"
#include <typeinfo>
#include "hash.h"
#include "EpsilonGreedyExplorer.h"
#include "TauFirstExplorer.h"
#include "SoftMaxExplorer.h"
#include "BaggingExplorer.h"

class MWTExplorer
{
public:
	MWTExplorer(std::string& app_id) : m_app_id(app_id)
	{
		IdGenerator::Initialize();

		if (m_app_id.empty())
		{
			m_app_id = this->Generate_App_Id();
		}

		m_logger = new Logger(m_app_id);
		m_explorer = nullptr;
		m_default_func_wrapper = nullptr;
	}

	~MWTExplorer()
	{
		IdGenerator::Destroy();

		if (m_logger)
			delete m_logger;
		if (m_explorer)
			delete m_explorer;
		if (m_action_set)
			delete m_action_set;
		if (m_default_func_wrapper)
			delete m_default_func_wrapper;
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
		std::tuple<MWTAction, float, bool> action_Probability_Log_Tuple = m_explorer->Choose_Action(context, *m_action_set);
		if(!std::get<2>(action_Probability_Log_Tuple)){
			return std::pair<u32, u64>(std::get<0>(action_Probability_Log_Tuple).Get_Id(), NO_JOIN_KEY);
		}
		Interaction pInteraction(&context, std::get<0>(action_Probability_Log_Tuple), std::get<1>(action_Probability_Log_Tuple));
		m_logger->Store(&pInteraction);
		
		
		// TODO: Anything else to do here?

		return std::pair<u32, u64>(std::get<0>(action_Probability_Log_Tuple).Get_Id(), pInteraction.Get_Id());
	}

	// TODO: check whether char* could be std::string
	u32 Choose_Action(Context& context, std::string unique_id)
	{
		u32 seed = this->Compute_Seed(const_cast<char*>(unique_id.c_str()), unique_id.size());
		std::tuple<MWTAction, float, bool> action_Probability_Log_Tuple = m_explorer->Choose_Action(context, *m_action_set, seed);
		if (!std::get<2>(action_Probability_Log_Tuple)){
			return std::get<0>(action_Probability_Log_Tuple).Get_Id();
		}
		Interaction pInteraction(&context, std::get<0>(action_Probability_Log_Tuple), std::get<1>(action_Probability_Log_Tuple), seed);
		m_logger->Store(&pInteraction);


		// TODO: Anything else to do here?

		return std::get<0>(action_Probability_Log_Tuple).Get_Id();
	}

// Cross-language interface
public:
	void Initialize_Epsilon_Greedy(
		float epsilon, 
		Stateful_Policy_Func default_policy_func, 
		void* default_policy_func_argument, u32 num_actions)
	{
		m_action_set = new ActionSet(num_actions);

		StatefulFunctionWrapper<void>* func_Wrapper = new StatefulFunctionWrapper<void>();
		func_Wrapper->m_policy_function = default_policy_func;
		
		m_explorer = new EpsilonGreedyExplorer<void>(epsilon, *func_Wrapper, default_policy_func_argument);
		
		m_default_func_wrapper = func_Wrapper;
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
		
		m_default_func_wrapper = func_Wrapper;
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
		
		m_default_func_wrapper = func_wrapper;
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
		
		m_default_func_wrapper = func_wrapper;
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

		m_default_func_wrapper = func_Wrapper;
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

		m_default_func_wrapper = func_wrapper;
	}

	u32 Choose_Action(feature* context_features, size_t num_features, std::string* other_context, std::string unique_id)
	{
		Context context(context_features, num_features, other_context);
		return this->Choose_Action(context, unique_id);
	}

	std::string Get_All_Interactions()
	{
		return m_logger->Get_All_Interactions();
	}

	void Get_All_Interactions(size_t& num_interactions, Interaction**& interactions)
	{
		m_logger->Get_All_Interactions(num_interactions, interactions);
	}

private:
	// TODO: App ID + Interaction ID is the unique identifier
	// Users can specify a seed and we use it to generate app id for them
	// so we can guarantee uniqueness.
	std::string Generate_App_Id()
	{
		return ""; // TODO: implement
	}

	u32 Compute_Seed(char* unique_id, u32 length)
	{
		// TODO: change return type to u64, may need to revisit this hash function
		return ::uniform_hash(unique_id, length, 0);
	}

private:
	std::string m_app_id;
	Explorer* m_explorer;
	Logger* m_logger;
	ActionSet* m_action_set;
	BaseFunctionWrapper* m_default_func_wrapper;
};

#if FALSE

class MWTRewardReporter
{
public:
	MWTRewardReporter(std::string& app_id) : m_app_id(app_id)
	{

	}


private:
			std::string m_app_id;
			Explorer* m_explorer;
			Logger* m_logger;
			ActionSet* m_action_set;
			BaseFunctionWrapper* m_default_func_wrapper;

		}
	}
}

#endif

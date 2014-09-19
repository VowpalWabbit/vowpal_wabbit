//
// Main interface for clients to the MWT service.
//

#include "stdafx.h"
#include <typeinfo>
#include "hash.h"
#include "EpsilonGreedyExplorer.h"
#include "TauFirstExplorer.h"

class MWT
{
public:
	MWT(std::string& app_id, u32 num_actions) : m_app_id(app_id)
	{
		IdGenerator::Initialize();

		if (m_app_id.empty())
		{
			m_app_id = this->Generate_App_Id();
		}

		m_logger = new Logger(m_app_id);
		m_explorer = nullptr;
		m_action_set = new ActionSet(num_actions);
		m_default_func_wrapper = nullptr;
	}

	~MWT()
	{
		IdGenerator::Destroy();

		delete m_logger;
		delete m_explorer;
		delete m_action_set;
		delete m_default_func_wrapper;
	}

	template <class T>
	void Initialize_Epsilon_Greedy(
		float epsilon, 
		typename StatefulFunctionWrapper<T>::Policy_Func default_policy_func, 
		T* default_policy_func_state_context)
	{
		this->Initialize_Epsilon_Greedy(epsilon, (Stateful_Policy_Func*)default_policy_func, (void*)default_policy_func_state_context);
	}

	void Initialize_Epsilon_Greedy(
		float epsilon, 
		StatelessFunctionWrapper::Policy_Func default_policy_func)
	{
		this->Initialize_Epsilon_Greedy(epsilon, (Stateless_Policy_Func*)default_policy_func);
	}

	/* Tau-first initialization */
	template <class T>
	void Initialize_Tau_First(
		u32 tau, 
		typename StatefulFunctionWrapper<T>::Policy_Func default_policy_func, 
		T* default_policy_func_state_context)
	{
		this->Initialize_Tau_First(tau, (Stateful_Policy_Func*)default_policy_func, (void*)default_policy_func_state_context);
	}

	void Initialize_Tau_First(
		u32 tau, 
		StatelessFunctionWrapper::Policy_Func default_policy_func)
	{
		this->Initialize_Tau_First(tau, (Stateless_Policy_Func*)default_policy_func);
	}

	std::pair<u32, u64> Choose_Action_Join_Key(Context& context)
	{
		std::pair<MWTAction, float> action_Probability_Pair = m_explorer->Choose_Action(context, *m_action_set);
		Interaction pInteraction(&context, action_Probability_Pair.first, action_Probability_Pair.second);
		m_logger->Store(&pInteraction);
		
		// TODO: Anything else to do here?

		return std::pair<u32, u64>(action_Probability_Pair.first.Get_Id(), pInteraction.Get_Id());
	}

	// TODO: check whether char* could be std::string
	u32 Choose_Action(Context& context, char* unique_id, u32 length)
	{
		u32 seed = this->Compute_Seed(unique_id, length);

		std::pair<MWTAction, float> action_Probability_Pair = m_explorer->Choose_Action(context, *m_action_set, seed);
		Interaction pInteraction(&context, action_Probability_Pair.first, action_Probability_Pair.second, seed);
		m_logger->Store(&pInteraction);

		// TODO: Anything else to do here?

		return action_Probability_Pair.first.Get_Id();
	}

// Cross-language interface
public:
	void Initialize_Epsilon_Greedy(
		float epsilon, 
		Stateful_Policy_Func default_policy_func, 
		void* default_policy_func_argument)
	{
		StatefulFunctionWrapper<void>* func_Wrapper = new StatefulFunctionWrapper<void>();
		func_Wrapper->m_policy_function = default_policy_func;
		
		m_explorer = new EpsilonGreedyExplorer<void>(epsilon, *func_Wrapper, default_policy_func_argument);
		
		m_default_func_wrapper = func_Wrapper;
	}

	void Initialize_Epsilon_Greedy(
		float epsilon, 
		Stateless_Policy_Func default_policy_func)
	{
		StatelessFunctionWrapper* func_Wrapper = new StatelessFunctionWrapper();
		func_Wrapper->m_policy_function = default_policy_func;
		
		m_explorer = new EpsilonGreedyExplorer<MWT_Empty>(epsilon, *func_Wrapper, nullptr);
		
		m_default_func_wrapper = func_Wrapper;
	}

	void Initialize_Tau_First(
		u32 tau, 
		Stateful_Policy_Func default_policy_func, 
		void* default_policy_func_argument)
	{
		StatefulFunctionWrapper<void>* func_wrapper = new StatefulFunctionWrapper<void>();
		func_wrapper->m_policy_function = default_policy_func;
		
		m_explorer = new TauFirstExplorer<void>(tau, *func_wrapper, default_policy_func_argument);
		
		m_default_func_wrapper = func_wrapper;
	}

	void Initialize_Tau_First(
		u32 tau, 
		Stateless_Policy_Func default_policy_func)
	{
		StatelessFunctionWrapper* func_wrapper = new StatelessFunctionWrapper();
		func_wrapper->m_policy_function = default_policy_func;
		
		m_explorer = new TauFirstExplorer<MWT_Empty>(tau, *func_wrapper, nullptr);
		
		m_default_func_wrapper = func_wrapper;
	}

	u32 Choose_Action(feature* context_features, size_t num_features, std::string* other_context, char* unique_id, u32 length)
	{
		Context context(context_features, num_features, other_context);
		return this->Choose_Action(context, unique_id, length);
	}

	std::string Get_All_Interactions()
	{
		return m_logger->Get_All_Interactions();
	}

private:
	// TODO: App ID + Interaction ID is the unique identifier
	// Users can specify a seed and we use it to generate app id for them
	// so we can guarantee uniqueness.
	std::string Generate_App_Id()
	{
		return ""; // TODO: implement
	}

	u32 MWT::Compute_Seed(char* unique_id, u32 length)
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
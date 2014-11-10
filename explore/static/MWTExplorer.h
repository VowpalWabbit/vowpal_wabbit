//
// Main interface for clients to the MWT service.
//

#pragma once

#include "Common.h"
#include "Explorer.h"
#include <functional>
#include <tuple>

MWT_NAMESPACE {

template <class Ctx>
class MwtExplorer
{
public:
	MwtExplorer(std::string app_id, IRecorder<Ctx>& recorder) : m_recorder(recorder)
    {
		m_app_id = HashUtils::Compute_Id_Hash(app_id);
    }
 
    template <class Exp>
    u32 Choose_Action(Exp& explorer, string unique_key, Ctx& context)
    {
		u64 seed = HashUtils::Compute_Id_Hash(unique_key);

		std::tuple<MWTAction, float, bool> action_probability_log_tuple = explorer.Choose_Action(seed + m_app_id, context);

		u32 action = std::get<0>(action_probability_log_tuple).Get_Id();
		float prob = std::get<1>(action_probability_log_tuple);

		if (std::get<2>(action_probability_log_tuple))
		{
			m_recorder.Record(context, action, prob, unique_key);
		}
 
        return action;
    }
 
private:
    u64 m_app_id;
	IRecorder<Ctx>& m_recorder;
};

//
// Top-level internal API for exploration (randomized decision making).
//
class OldMWTExplorer
{
public:
	OldMWTExplorer(std::string app_id)
	{
		m_app_id = HashUtils::Compute_Id_Hash(app_id);
	}

	~OldMWTExplorer()
	{
	}

	template <class T>
	void Initialize_Epsilon_Greedy(
		float epsilon, 
		typename Stateful<T>::Policy default_policy_func, 
		T& default_policy_params, u32 num_actions)
	{
		this->Internal_Initialize_Epsilon_Greedy(epsilon, (Stateful_Policy_Func*)default_policy_func, (void*)&default_policy_params, num_actions);
	}

	void Initialize_Epsilon_Greedy(
		float epsilon, 
		Policy default_policy_func,
		u32 num_actions)
	{
		this->Internal_Initialize_Epsilon_Greedy(epsilon, (Stateless_Policy_Func*)default_policy_func, num_actions);
	}

	/* Tau-first initialization */
	template <class T>
	void Initialize_Tau_First(
		u32 tau, 
		typename Stateful<T>::Policy default_policy_func, 
		T& default_policy_params,
		u32 num_actions)
	{
		this->Internal_Initialize_Tau_First(tau, (Stateful_Policy_Func*)default_policy_func, (void*)&default_policy_params, num_actions);
	}

	void Initialize_Tau_First(
		u32 tau, 
		Policy default_policy_func,
		u32 num_actions)
	{
		this->Internal_Initialize_Tau_First(tau, (Stateless_Policy_Func*)default_policy_func, num_actions);
	}

	/* Bagging initialization */
	template <class T>
	void Initialize_Bagging(
		u32 bags,
		typename Stateful<T>::Policy* default_policy_functions[],
		T* default_policy_params[],
		u32 num_actions)
	{
		this->Internal_Initialize_Bagging(bags, (Stateful_Policy_Func**)default_policy_functions, (void**)default_policy_params, num_actions);
	}

	void Initialize_Bagging(
		u32 bags,
		Policy* default_policy_functions[],
		u32 num_actions)
	{
		this->Internal_Initialize_Bagging(bags, (Stateless_Policy_Func**)default_policy_functions, num_actions);
	}

	/* Softmax initialization */
	template <class T>
	void Initialize_Softmax(
		float lambda,
		typename Stateful<T>::Scorer default_scorer_func,
		T& default_scorer_params, u32 num_actions)
	{
		this->Internal_Initialize_Softmax(lambda, (Stateful_Scorer_Func*)default_scorer_func, (void*)&default_scorer_params, num_actions);
	}

	void Initialize_Softmax(
		float lambda,
		Scorer default_scorer_func,
		u32 num_actions)
	{
		this->Internal_Initialize_Softmax(lambda, (Stateless_Scorer_Func*)default_scorer_func, num_actions);
	}

	/* Generic initialization */
	template <class T>
	void Initialize_Generic(
		typename Stateful<T>::Scorer default_scorer_func,
		T& default_scorer_params, u32 num_actions)
	{
		this->Internal_Initialize_Generic((Stateful_Scorer_Func*)default_scorer_func, (void*)&default_scorer_params, num_actions);
	}

	void Initialize_Generic(
		Scorer default_scorer_func, u32 num_actions)
	{
		this->Internal_Initialize_Generic((Stateless_Scorer_Func*)default_scorer_func, num_actions);
	}

	//TODO: Mention that this allocates memory, so it may throw if contiguous address space is not available.
	u32 Choose_Action(std::string unique_id, BaseContext& context)
	{
		return this->Internal_Choose_Action(context, unique_id);
	}

	//TODO: Mention that this will clear the interactions from our explorer's internal memory
	inline std::string Get_All_Interactions_As_String()
	{
		return m_interaction_store.Get_All_Interactions_As_String();
	}

	std::vector<Interaction> Get_All_Interactions()
	{
		return m_interaction_store.Get_All_Interactions();
	}

// Cross-language interface
PORTING_INTERFACE:
	void Internal_Initialize_Epsilon_Greedy(
		float epsilon, 
		Stateful_Policy_Func default_policy_func, 
		void* default_policy_func_argument, 
		u32 num_actions)
	{
		Validate_Epsilon(epsilon);
		Validate_Num_Actions(num_actions);
		Validate_Policy<Stateful_Policy_Func>(default_policy_func);
		Validate_Explorer_Empty();

		m_action_set.Set_Count(num_actions);
		m_explorer.reset(new OldEpsilonGreedyExplorer(epsilon, default_policy_func, default_policy_func_argument, m_app_id));
	}

	void Internal_Initialize_Epsilon_Greedy(
		float epsilon, 
		Stateless_Policy_Func default_policy_func,
		u32 num_actions)
	{
		Validate_Epsilon(epsilon);
		Validate_Num_Actions(num_actions);
		Validate_Policy<Stateless_Policy_Func>(default_policy_func);
		Validate_Explorer_Empty();

		m_action_set.Set_Count(num_actions);
		m_explorer.reset(new OldEpsilonGreedyExplorer(epsilon, default_policy_func, m_app_id));
	}

	void Internal_Initialize_Tau_First(
		u32 tau, 
		Stateful_Policy_Func default_policy_func, 
		void* default_policy_func_argument,
		u32 num_actions)
	{
		Validate_Num_Actions(num_actions);
		Validate_Policy<Stateful_Policy_Func>(default_policy_func);
		Validate_Explorer_Empty();

		m_action_set.Set_Count(num_actions);
		m_explorer.reset(new OldTauFirstExplorer(tau, default_policy_func, default_policy_func_argument, m_app_id));
	}

	void Internal_Initialize_Tau_First(
		u32 tau, 
		Stateless_Policy_Func default_policy_func,
		u32 num_actions)
	{
		Validate_Num_Actions(num_actions);
		Validate_Policy<Stateless_Policy_Func>(default_policy_func);
		Validate_Explorer_Empty();
		
		m_action_set.Set_Count(num_actions);
		m_explorer.reset(new OldTauFirstExplorer(tau, default_policy_func, m_app_id));
	}

	void Internal_Initialize_Bagging(
		u32 bags,
		Stateful_Policy_Func** default_policy_functions,
		void** default_policy_args,
		u32 num_actions)
	{
		Validate_Bags(bags);
		Validate_Num_Actions(num_actions);
		Validate_Policy<Stateful_Policy_Func>(bags, default_policy_functions);
		Validate_Explorer_Empty();
		
		m_action_set.Set_Count(num_actions);
		m_explorer.reset(new OldBaggingExplorer(bags, default_policy_functions, default_policy_args, m_app_id));
	}

	void Internal_Initialize_Bagging(
		u32 bags,
		Stateless_Policy_Func** default_policy_functions,
		u32 num_actions)
	{
		Validate_Bags(bags);
		Validate_Num_Actions(num_actions);
		Validate_Policy<Stateless_Policy_Func>(bags, default_policy_functions);
		Validate_Explorer_Empty();
		
		m_action_set.Set_Count(num_actions);
		m_explorer.reset(new OldBaggingExplorer(bags, default_policy_functions, m_app_id));
	}

	void Internal_Initialize_Softmax(
		float lambda,
		Stateful_Scorer_Func default_scorer_func,
		void* default_scorer_func_argument,
		u32 num_actions)
	{
		Validate_Num_Actions(num_actions);
		Validate_Policy<Stateful_Scorer_Func>(default_scorer_func);
		Validate_Explorer_Empty();
		
		m_action_set.Set_Count(num_actions);
		m_explorer.reset(new OldSoftmaxExplorer(lambda, default_scorer_func, default_scorer_func_argument, m_app_id));
	}

	void Internal_Initialize_Softmax(
		float lambda,
		Stateless_Scorer_Func default_scorer_func,
		u32 num_actions)
	{
		Validate_Num_Actions(num_actions);
		Validate_Policy<Stateless_Scorer_Func>(default_scorer_func);
		Validate_Explorer_Empty();
		
		m_action_set.Set_Count(num_actions);
		m_explorer.reset(new OldSoftmaxExplorer(lambda, default_scorer_func, m_app_id));
	}

	void Internal_Initialize_Generic(
		Stateful_Scorer_Func default_scorer_func,
		void* default_scorer_func_argument,
		u32 num_actions)
	{
		Validate_Num_Actions(num_actions);
		Validate_Policy<Stateful_Scorer_Func>(default_scorer_func);
		Validate_Explorer_Empty();
		
		m_action_set.Set_Count(num_actions);
		m_explorer.reset(new OldGenericExplorer(default_scorer_func, default_scorer_func_argument, m_app_id));
	}

	void Internal_Initialize_Generic(
		Stateless_Scorer_Func default_scorer_func,
		u32 num_actions)
	{
		Validate_Num_Actions(num_actions);
		Validate_Policy<Stateless_Scorer_Func>(default_scorer_func);
		Validate_Explorer_Empty();
		
		m_action_set.Set_Count(num_actions);
		m_explorer.reset(new OldGenericExplorer(default_scorer_func, m_app_id));
	}
	
	u32 Internal_Choose_Action(BaseContext& context, std::string unique_id)
	{
		Validate_Explorer_Present();

		// Hash the ID of the yet-to-be-created interaction so we can seed the explorer
		u64 seed = HashUtils::Compute_Id_Hash(unique_id);
		std::tuple<MWTAction, float, bool> action_Probability_Log_Tuple = m_explorer->Choose_Action((void*)&context, m_action_set, (u32)seed);
		
		if (std::get<2>(action_Probability_Log_Tuple))
		{
			// Create an interaction using the same unique_id as used in the seed above!
			Interaction interaction(&context, std::get<0>(action_Probability_Log_Tuple), std::get<1>(action_Probability_Log_Tuple), unique_id);
			m_interaction_store.Store(interaction);
		}
		return std::get<0>(action_Probability_Log_Tuple).Get_Id();
	}

#ifdef MANAGED_CODE
	u32 Interop_Choose_Action(BaseContext& log_context, std::string unique_id, void* clr_context)
	{
		Validate_Explorer_Present();

		// Hash the ID of the yet-to-be-created interaction so we can seed the explorer
		u64 seed = HashUtils::Compute_Id_Hash(unique_id);
		std::tuple<MWTAction, float, bool> action_Probability_Log_Tuple = m_explorer->Choose_Action(clr_context, m_action_set, (u32)seed);

		if (std::get<2>(action_Probability_Log_Tuple))
		{
			// Create an interaction using the same unique_id as used in the seed above!
			Interaction interaction(&log_context, std::get<0>(action_Probability_Log_Tuple), std::get<1>(action_Probability_Log_Tuple), unique_id);
			interaction.Set_Clr_Context(clr_context);
			m_interaction_store.Store(interaction);
		}
		return std::get<0>(action_Probability_Log_Tuple).Get_Id();
	}
#endif

private:
	void Validate_Num_Actions(u32 num_actions)
	{
		if (num_actions < 1)
		{
			throw std::invalid_argument("Number of actions must be at least 1.");
		}
	}
	void Validate_Epsilon(float epsilon)
	{
		if (epsilon < 0 || epsilon > 1)
		{
			throw std::invalid_argument("Epsilon must be between 0 and 1.");
		}
	}
	void Validate_Bags(u32 bags)
	{
		if (bags < 1)
		{
			throw std::invalid_argument("Number of bags must be at least 1.");
		}
	}
	template <class T>
	void Validate_Policy(T* policy)
	{
		if (policy == nullptr)
		{
			throw std::invalid_argument("A valid default policy function must be specified.");
		}
	}
	template <class T>
	void Validate_Policy(u32 num_policies, T** policies)
	{
		bool valid = true;
		if (policies == nullptr)
		{
			valid = false;
		}
		else
		{
			for (u32 i = 0; i < num_policies; i++)
			{
				if (policies[i] == nullptr)
				{
					valid = false;
					break;
				}
			}
		}
		if (!valid)
		{
			throw std::invalid_argument("Invalid default policy functions specified.");
		}
	}

	void Validate_Explorer_Empty()
	{
		if (m_explorer.get() != nullptr)
		{
			throw std::invalid_argument("MWT is already initialized.");
		}
	}

	void Validate_Explorer_Present()
	{
		if (m_explorer.get() == nullptr)
		{
			throw std::invalid_argument("MWT was not initialized properly.");
		}
	}

private:
	unique_ptr<Explorer> m_explorer;
	InteractionStore m_interaction_store;
	ActionSet m_action_set;
	u64 m_app_id;
};
}

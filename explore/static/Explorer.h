#pragma once

#include <float.h>
#include <functional>
#include <tuple>
#include <math.h>
#include "Interaction.h"

//
// Common interface for all exploration algorithms
//

/*  TODO: clean up these comments
	These classes are used internally within OldMwtExplorer.h. 
	Behavior of independent external usage is undefined. 
*/
MWT_NAMESPACE {

template <class Rec>
class MwtExplorer;

template <class Ctx>
class IRecorder
{
public:
	virtual void Record(Ctx& context, u32 action, float probability, string unique_key) = 0;
};

template <class Ctx>
class IPolicy
{
public:
	virtual u32 Choose_Action(Ctx& context) = 0;
};

template <class Ctx>
class IScorer
{
public:
	virtual vector<float> Score_Actions(Ctx& context) = 0;
};

class Explorer
{
public:
	virtual std::tuple<MWTAction, float, bool> Choose_Action(void* context, ActionSet& actions, u32 seed) = 0;
	virtual ~Explorer() { }
};

// Default Recorder that converts tuple into string format.
template <class Ctx>
struct StringRecorder : public IRecorder<Ctx>
{
	void Record(Ctx& context, u32 action, float probability, string unique_key)
	{
		// Implicitly enforce To_String() API on the context
		m_recording.append(to_string(action));
		m_recording.append(" ", 1);
		m_recording.append(unique_key);
		m_recording.append(" ", 1);

		char prob_str[10] = { 0 };
		NumberUtils::Float_To_String(probability, prob_str);
		m_recording.append(prob_str);

		m_recording.append(" | ", 3);
		m_recording.append(context.To_String());
		m_recording.append("\n");
	}

	string Get_Recording()
	{
		return m_recording;
	}

private:
	string m_recording;
};

class SimpleContext
{
public:
	SimpleContext(vector<Feature>& features) :
		m_features(features)
	{ }

	vector<Feature>& Get_Features()
	{
		return m_features;
	}

	string To_String()
	{
		string out_string;
		char feature_str[35] = { 0 };
		for (size_t i = 0; i < m_features.size(); i++)
		{
			int chars;
			if (i == 0)
			{
				chars = sprintf(feature_str, "%d:", m_features[i].Id);
			}
			else
			{
				chars = sprintf(feature_str, " %d:", m_features[i].Id);
			}
			NumberUtils::print_float(feature_str + chars, m_features[i].Value);
			out_string.append(feature_str);
		}
		return out_string;
	}

private:
	vector<Feature>& m_features;
};

template <class Plc>
class EpsilonGreedyExplorer
{
public:
	EpsilonGreedyExplorer(Plc& default_policy, float epsilon, u32 num_actions) :
		m_default_policy(default_policy), m_epsilon(epsilon), m_num_actions(num_actions)
	{
		if (m_num_actions < 1)
		{
			throw std::invalid_argument("Number of actions must be at least 1.");
		}

		if (m_epsilon < 0 || m_epsilon > 1)
		{
			throw std::invalid_argument("Epsilon must be between 0 and 1.");
		}
	}

	~EpsilonGreedyExplorer()
	{
	}

private:
	template <class Ctx>
	std::tuple<MWTAction, float, bool> Choose_Action(u64 salted_seed, Ctx& context)
	{
		ActionSet actions;
		actions.Set_Count(m_num_actions);

		PRG::prg random_generator(salted_seed);

		// Invoke the default policy function to get the action
		static_assert(std::is_base_of<IPolicy<Ctx>, Plc>::value, "The specified policy does not implement IPolicy");
		IPolicy<Ctx>* policy = (IPolicy<Ctx>*)&m_default_policy;

		MWTAction chosen_action = MWTAction(policy->Choose_Action(context));

		if (chosen_action.Get_Id() == 0 || chosen_action.Get_Id() > actions.Count())
		{
			throw std::invalid_argument("Action chosen by default policy is not within valid range.");
		}

		float action_probability = 0.f;
		float base_probability = m_epsilon / actions.Count(); // uniform probability

		// TODO: check this random generation
		if (random_generator.Uniform_Unit_Interval() < 1.f - m_epsilon)
		{
			action_probability = 1.f - m_epsilon + base_probability;
		}
		else
		{
			// Get uniform random action ID
			u32 actionId = random_generator.Uniform_Int(1, actions.Count());

			if (actionId == chosen_action.Get_Id())
			{
				// IF it matches the one chosen by the default policy
				// then increase the probability
				action_probability = 1.f - m_epsilon + base_probability;
			}
			else
			{
				// Otherwise it's just the uniform probability
				action_probability = base_probability;
			}
			chosen_action = actions.Get(actionId);
		}

		return std::tuple<MWTAction, float, bool>(chosen_action, action_probability, true);
	}

private:
	Plc& m_default_policy;
	float m_epsilon;
	u32 m_num_actions;

private:
	template <class Rec>
	friend class MwtExplorer;
};

template <class Scr>
class SoftmaxExplorer
{
public:
	SoftmaxExplorer(Scr& default_scorer, float lambda, u32 num_actions) :
		m_default_scorer(default_scorer), m_lambda(lambda), m_num_actions(num_actions)
	{
	}

private:
	template <class Ctx>
	std::tuple<MWTAction, float, bool> Choose_Action(u64 salted_seed, Ctx& context)
	{
		ActionSet actions;
		actions.Set_Count(m_num_actions);
		PRG::prg random_generator(salted_seed);

		// Invoke the default scorer function
		static_assert(std::is_base_of<IScorer<Ctx>, Scr>::value, "The specified scorer does not implement IScorer");
		IScorer<Ctx>* scorer = (IScorer<Ctx>*)&m_default_scorer;

		vector<float> scores = scorer->Score_Actions(context);
		u32 num_scores = (u32)scores.size();
		if (num_scores != m_num_actions)
		{
			throw std::invalid_argument("The number of scores returned by the scorer must equal number of actions");
		}

		MWTAction chosen_action(0);

		u32 i = 0;

		float max_score = -FLT_MAX;
		for (i = 0; i < num_scores; i++)
		{
			if (max_score < scores[i])
			{
				max_score = scores[i];
			}
		}

		// Create a normalized exponential distribution based on the returned scores
		for (i = 0; i < num_scores; i++)
		{
			scores[i] = exp(m_lambda * (scores[i] - max_score));
		}

		// Create a discrete_distribution based on the returned weights. This class handles the
		// case where the sum of the weights is < or > 1, by normalizing agains the sum.
		float total = 0.f;
		for (size_t i = 0; i < num_scores; i++)
			total += scores[i];

		float draw = random_generator.Uniform_Unit_Interval();

		float sum = 0.f;
		float action_probability = 0.f;
		u32 action_index = num_scores - 1;
		for (u32 i = 0; i < num_scores; i++)
		{
			scores[i] = scores[i] / total;
			sum += scores[i];
			if (sum > draw)
			{
				action_index = i;
				action_probability = scores[i];
				break;
			}
		}

		return std::tuple<MWTAction, float, bool>(actions.Get(MWTAction::Make_OneBased(action_index)), action_probability, true);
	}

private:
	Scr& m_default_scorer;
	float m_lambda;
	u32 m_num_actions;

private:
	template <class Rec>
	friend class MwtExplorer;
};

template <class Scr>
class GenericExplorer
{
public:
	GenericExplorer(Scr& default_scorer, u32 num_actions) : 
		m_default_scorer(default_scorer), m_num_actions(num_actions)
	{
	}

private:
	template <class Ctx>
	std::tuple<MWTAction, float, bool> Choose_Action(u64 salted_seed, Ctx& context)
	{
		ActionSet actions;
		actions.Set_Count(m_num_actions);
		PRG::prg random_generator(salted_seed);

		// Invoke the default scorer function
		static_assert(std::is_base_of<IScorer<Ctx>, Scr>::value, "The specified scorer does not implement IScorer");
		IScorer<Ctx>* scorer = (IScorer<Ctx>*)&m_default_scorer;

		vector<float> weights = scorer->Score_Actions(context);
		u32 num_weights = (u32)weights.size();
		if (num_weights != m_num_actions)
		{
			throw std::invalid_argument("The number of weights returned by the scorer must equal number of actions");
		}

		MWTAction chosen_action(0);

		// Create a discrete_distribution based on the returned weights. This class handles the
		// case where the sum of the weights is < or > 1, by normalizing agains the sum.
		float total = 0.f;
		for (size_t i = 0; i < num_weights; i++)
		{
			if (weights[i] < 0)
			{
				throw std::invalid_argument("Scores must be non-negative.");
			}
			total += weights[i];
		}
		if (total == 0)
		{
			throw std::invalid_argument("At least one score must be positive.");
		}

		float draw = random_generator.Uniform_Unit_Interval();

		float sum = 0.f;
		float action_probability = 0.f;
		u32 action_index = num_weights - 1;
		for (u32 i = 0; i < num_weights; i++)
		{
			weights[i] = weights[i] / total;
			sum += weights[i];
			if (sum > draw)
			{
				action_index = i;
				action_probability = weights[i];
				break;
			}
		}

		return std::tuple<MWTAction, float, bool>(actions.Get(MWTAction::Make_OneBased(action_index)), action_probability, true);
	}

private:
	Scr& m_default_scorer;
	u32 m_num_actions;

private:
	template <class Rec>
	friend class MwtExplorer;
};

template <class Plc>
class TauFirstExplorer
{
public:
	TauFirstExplorer(Plc& default_policy, u32 tau, u32 num_actions) :
		m_default_policy(default_policy), m_tau(tau), m_num_actions(num_actions)
	{
	}

	template <class Ctx>
	std::tuple<MWTAction, float, bool> Choose_Action(u64 salted_seed, Ctx& context)
	{
		ActionSet actions;
		actions.Set_Count(m_num_actions);

		PRG::prg random_generator(salted_seed);

		MWTAction chosen_action(0);
		float action_probability = 0.f;
		bool log_action;
		if (m_tau)
		{
			m_tau--;
			u32 actionId = random_generator.Uniform_Int(1, actions.Count());
			action_probability = 1.f / actions.Count();
			chosen_action = actions.Get(actionId);
			log_action = true;
		}
		else
		{
			// Invoke the default policy function to get the action
			static_assert(std::is_base_of<IPolicy<Ctx>, Plc>::value, "The specified policy does not implement IPolicy");
			IPolicy<Ctx>* policy = (IPolicy<Ctx>*)&m_default_policy;

			MWTAction chosen_action = MWTAction(policy->Choose_Action(context));

			if (chosen_action.Get_Id() == 0 || chosen_action.Get_Id() > actions.Count())
			{
				throw std::invalid_argument("Action chosen by default policy is not within valid range.");
			}

			action_probability = 1.f;
			log_action = false;
		}

		return std::tuple<MWTAction, float, bool>(chosen_action, action_probability, log_action);
	}

private:
	Plc& m_default_policy;
	u32 m_tau;
	u32 m_num_actions;

private:
	template <class Rec>
	friend class MwtExplorer;
};

template <class Plc>
class BaggingExplorer
{
public:
	BaggingExplorer(vector<Plc>& default_policy_functions, u32 bags, u32 num_actions) :
		m_default_policy_functions(default_policy_functions),
		m_bags(bags),
		m_num_actions(num_actions)
	{
	}

	template <class Ctx>
	std::tuple<MWTAction, float, bool> Choose_Action(u64 salted_seed, Ctx& context)
	{
		ActionSet actions;
		actions.Set_Count(m_num_actions);

		PRG::prg random_generator(salted_seed);

		// Select bag
		u32 chosen_bag = random_generator.Uniform_Int(0, m_bags - 1);

		// Invoke the default policy function to get the action
		MWTAction chosen_action(0);
		MWTAction action_from_bag(0);
		vector<u32> actions_selected;
		for (size_t i = 0; i < actions.Count(); i++)
		{
			actions_selected.push_back(0);
		}

		// Invoke the default policy function to get the action
		static_assert(std::is_base_of<IPolicy<Ctx>, Plc>::value, "The specified policy does not implement IPolicy");

		for (u32 current_bag = 0; current_bag < m_bags; current_bag++)
		{
			IPolicy<Ctx>* policy = (IPolicy<Ctx>*)&m_default_policy_functions[current_bag];
			action_from_bag = MWTAction(policy->Choose_Action(context));

			if (action_from_bag.Get_Id() == 0 || action_from_bag.Get_Id() > actions.Count())
			{
				throw std::invalid_argument("Action chosen by default policy is not within valid range.");
			}

			if (current_bag == chosen_bag)
			{
				chosen_action = action_from_bag;
			}
			//this won't work if actions aren't 0 to Count
			actions_selected[action_from_bag.Get_Id_ZeroBased()]++;
		}
		float action_probability = (float)actions_selected[chosen_action.Get_Id_ZeroBased()] / m_bags;

		return std::tuple<MWTAction, float, bool>(chosen_action, action_probability, true);
	}

private:
	vector<Plc>& m_default_policy_functions;
	u32 m_bags;
	u32 m_num_actions;

private:
	template <class Rec>
	friend class MwtExplorer;
};

class OldEpsilonGreedyExplorer : public Explorer
{
public:
	OldEpsilonGreedyExplorer(
		float epsilon,
		Stateful_Policy_Func* default_policy_func,
		void* default_policy_params,
		u64 salt) :
                m_epsilon(epsilon),
		m_salt(salt),
		m_stateful_default_policy_func(default_policy_func),
		m_stateless_default_policy_func(nullptr),
		m_default_policy_params(default_policy_params)
	{
	}

	OldEpsilonGreedyExplorer(
		float epsilon,
		Stateless_Policy_Func* default_policy_func,
		u64 salt) :
		m_epsilon(epsilon),
		m_salt(salt),
		m_stateful_default_policy_func(nullptr),
		m_stateless_default_policy_func(default_policy_func),
		m_default_policy_params(nullptr)
	{
	}

	~OldEpsilonGreedyExplorer()
	{
	}

	std::tuple<MWTAction, float, bool> Choose_Action(void* context, ActionSet& actions, u32 seed)
	{
		PRG::prg random_generator(m_salt + seed);
		// Invoke the default policy function to get the action
		MWTAction chosen_action(0);
		if (m_stateless_default_policy_func != nullptr)
		{
			chosen_action = MWTAction(m_stateless_default_policy_func(context));
		}
		else
		{
			chosen_action = MWTAction(m_stateful_default_policy_func(m_default_policy_params, context));
		}

		if (chosen_action.Get_Id() == 0 || chosen_action.Get_Id() > actions.Count())
		{
			throw std::invalid_argument("Action chosen by default policy is not within valid range.");
		}

		float action_probability = 0.f;
		float base_probability = m_epsilon / actions.Count(); // uniform probability

		// TODO: check this random generation
		if (random_generator.Uniform_Unit_Interval() < 1.f - m_epsilon)
		{
			action_probability = 1.f - m_epsilon + base_probability;
		}
		else
		{
			// Get uniform random action ID
			u32 actionId = random_generator.Uniform_Int(1, actions.Count());

			if (actionId == chosen_action.Get_Id())
			{
				// IF it matches the one chosen by the default policy
				// then increase the probability
				action_probability = 1.f - m_epsilon + base_probability;
			}
			else
			{
				// Otherwise it's just the uniform probability
				action_probability = base_probability;
			}
			chosen_action = actions.Get(actionId);
		}

		return std::tuple<MWTAction, float, bool>(chosen_action, action_probability, true);
	}

private:
	float m_epsilon;
	u64 m_salt;

	Stateful_Policy_Func* m_stateful_default_policy_func;
	Stateless_Policy_Func* m_stateless_default_policy_func;
	void* m_default_policy_params;
};


class OldSoftmaxExplorer : public Explorer
{
public:
	OldSoftmaxExplorer(
		float lambda,
		Stateful_Scorer_Func* default_scorer_func,
		void* default_scorer_params,
		u64 salt) :
		m_lambda(lambda),
		m_salt(salt),
		m_stateful_default_scorer_func(default_scorer_func),
		m_stateless_default_scorer_func(nullptr),
		m_default_scorer_params(default_scorer_params)
	{
	}

	OldSoftmaxExplorer(
		float lambda,
		Stateless_Scorer_Func* default_scorer_func,
		u64 salt) :
		m_lambda(lambda),
		m_salt(salt),
		m_stateful_default_scorer_func(nullptr),
		m_stateless_default_scorer_func(default_scorer_func),
		m_default_scorer_params(nullptr)
	{
	}

	std::tuple<MWTAction, float, bool> Choose_Action(void* context, ActionSet& actions, u32 seed)
	{
		PRG::prg random_generator(seed);
		MWTAction chosen_action(0);
		u32 numScores = actions.Count();
		std::unique_ptr<float[]> scores(new float[numScores]());
		// Invoke the default scorer function to score each action 
		if (m_stateless_default_scorer_func != nullptr)
		{
			m_stateless_default_scorer_func(context, scores.get(), actions.Count());
		}
		else
		{
			m_stateful_default_scorer_func(m_default_scorer_params, context, scores.get(), actions.Count());
		}

		u32 i = 0;

		float max_score = -FLT_MAX;
		for (i = 0; i < numScores; i++)
		{
			if (max_score < scores[i])
			{
				max_score = scores[i];
			}
		}

		// Create a normalized exponential distribution based on the returned scores
		for (i = 0; i < numScores; i++)
		{
			scores[i] = exp(m_lambda * (scores[i] - max_score));
		}

		// Create a discrete_distribution based on the returned weights. This class handles the
		// case where the sum of the weights is < or > 1, by normalizing agains the sum.
		float total = 0.f;
		for (size_t i = 0; i < numScores; i++)
		  total += scores[i];
		
		float draw = random_generator.Uniform_Unit_Interval();
		
		float sum = 0.f;
		float action_probability = 0.f;
		u32 action_index = numScores - 1;
		for (u32 i = 0; i < numScores; i++)
		{
			scores[i] = scores[i] / total;
			sum += scores[i];
			if (sum > draw)
			{
				action_index = i;
				action_probability = scores[i];
				break;
			}
		}

		return std::tuple<MWTAction, float, bool>(actions.Get(MWTAction::Make_OneBased(action_index)), action_probability, true);
 	}

private:
	float m_lambda;
	u64 m_salt;

	Stateful_Scorer_Func* m_stateful_default_scorer_func;
	Stateless_Scorer_Func* m_stateless_default_scorer_func;
	void* m_default_scorer_params;
};


class OldGenericExplorer : public Explorer
{
public:
	OldGenericExplorer(
		Stateful_Scorer_Func* default_scorer_func, 
		void* default_scorer_params,
		u64 salt) :
                m_salt(salt),
		m_stateful_default_scorer_func(default_scorer_func),
		m_stateless_default_scorer_func(nullptr),
		m_default_scorer_params(default_scorer_params)
	{
	}

	OldGenericExplorer(
		Stateless_Scorer_Func* default_scorer_func,
		u64 salt) :
		m_salt(salt),
		m_stateful_default_scorer_func(nullptr),
		m_stateless_default_scorer_func(default_scorer_func),
		m_default_scorer_params(nullptr)
	{
	}

	std::tuple<MWTAction, float, bool> Choose_Action(void* context, ActionSet& actions, u32 seed)
	{
		PRG::prg random_generator(seed);
		MWTAction chosen_action(0);
		u32 numWeights = actions.Count();
		std::unique_ptr<float[]> weights(new float[numWeights]());
		// Invoke the default scorer function to get the weight of each action 
		if (m_stateless_default_scorer_func != nullptr)
		{
			m_stateless_default_scorer_func(context, weights.get(), actions.Count());
		}
		else
		{
			m_stateful_default_scorer_func(m_default_scorer_params, context, weights.get(), actions.Count());
		}

		// Create a discrete_distribution based on the returned weights. This class handles the
		// case where the sum of the weights is < or > 1, by normalizing agains the sum.
		float total = 0.f;
		for (size_t i = 0; i < numWeights; i++)
		{
			if (weights[i] < 0)
			{
				throw std::invalid_argument("Scores must be non-negative.");
			}
			total += weights[i];
		}
		if (total == 0)
		{
			throw std::invalid_argument("At least one score must be positive.");
		}

		float draw = random_generator.Uniform_Unit_Interval();
		
		float sum = 0.f;
		float action_probability = 0.f;
		u32 action_index = numWeights-1;
		for (u32 i = 0; i < numWeights; i++)
		{
			weights[i] = weights[i] / total;
			sum += weights[i];
			if (sum > draw)
			{
				action_index = i;
				action_probability = weights[i];
				break;
			}
		}

		return std::tuple<MWTAction, float, bool>(actions.Get(MWTAction::Make_OneBased(action_index)), action_probability, true);
	}

private:
	u64 m_salt;

	Stateful_Scorer_Func* m_stateful_default_scorer_func;
	Stateless_Scorer_Func* m_stateless_default_scorer_func;
	void* m_default_scorer_params;
};


class OldTauFirstExplorer : public Explorer
{
public:
	OldTauFirstExplorer(
		u32 tau,
		Stateful_Policy_Func* default_policy_func,
		void* default_policy_params,
		u64 salt) :
		m_tau(tau),
		m_salt(salt),
		m_stateful_default_policy_func(default_policy_func),
		m_stateless_default_policy_func(nullptr),
		m_default_policy_params(default_policy_params)
	{
	}

	OldTauFirstExplorer(
		u32 tau,
		Stateless_Policy_Func* default_policy_func,
		u64 salt) :
		m_tau(tau),
		m_salt(salt),
		m_stateful_default_policy_func(nullptr),
		m_stateless_default_policy_func(default_policy_func),
		m_default_policy_params(nullptr)
	{
	}

	std::tuple<MWTAction, float, bool> Choose_Action(void* context, ActionSet& actions, u32 seed)
	{
		PRG::prg random_generator(seed + m_salt);
		MWTAction chosen_action(0);
		float action_probability = 0.f;
		bool log_action;
		if (m_tau)
		{
			m_tau--;
			u32 actionId = random_generator.Uniform_Int(1, actions.Count());
			action_probability = 1.f / actions.Count();
			chosen_action = actions.Get(actionId);
			log_action = true;
		}
		else
		{
			// Invoke the default policy function to get the action
			if (m_stateless_default_policy_func != nullptr)
			{
				chosen_action = MWTAction(m_stateless_default_policy_func(context));
			}
			else
			{
				chosen_action = MWTAction(m_stateful_default_policy_func(m_default_policy_params, context));
			}

			if (chosen_action.Get_Id() == 0 || chosen_action.Get_Id() > actions.Count())
			{
				throw std::invalid_argument("Action chosen by default policy is not within valid range.");
			}

			action_probability = 1.f;
			log_action = false;
		}

		return std::tuple<MWTAction, float, bool>(chosen_action, action_probability, log_action);
	}

private:
	u32 m_tau;
	u64 m_salt;

	Stateful_Policy_Func* m_stateful_default_policy_func;
	Stateless_Policy_Func* m_stateless_default_policy_func;
	void* m_default_policy_params;
};


class OldBaggingExplorer : public Explorer
{
public:
	OldBaggingExplorer(
		u32 bags,
		Stateful_Policy_Func** default_policy_functions,
		void** default_policy_args,
		u64 salt) :
		m_bags(bags),
		m_salt(salt),
		m_stateful_default_policy_funcs(default_policy_functions),
		m_stateless_default_policy_funcs(nullptr),
		m_default_policy_params(default_policy_args)
	{
	}

	OldBaggingExplorer(
		u32 bags,
		Stateless_Policy_Func** default_policy_functions,
		u64 salt) :
		m_bags(bags),
		m_salt(salt),
		m_stateful_default_policy_funcs(nullptr),
		m_stateless_default_policy_funcs(default_policy_functions),
		m_default_policy_params(nullptr)
	{
	}

	~OldBaggingExplorer()
	{
	}

	std::tuple<MWTAction, float, bool> Choose_Action(void* context, ActionSet& actions, u32 seed)
	{
		PRG::prg random_generator(seed + m_salt);
		// Select bag
		u32 chosen_bag = random_generator.Uniform_Int(0, m_bags - 1);
		// Invoke the default policy function to get the action
		MWTAction chosen_action(0);
		MWTAction action_from_bag(0);
		// Maybe be best to make this static size
		std::unique_ptr<u32[]> actions_selected(new u32[actions.Count()]);
		for (size_t i = 0; i < actions.Count(); i++)
		{
			actions_selected[i] = 0;
		}
		for (u32 current_bag = 0; current_bag < m_bags; current_bag++)
		{
			if (m_stateless_default_policy_funcs != nullptr)
			{
				action_from_bag = MWTAction(m_stateless_default_policy_funcs[current_bag](context));
			}
			else
			{
				action_from_bag = MWTAction(m_stateful_default_policy_funcs[current_bag](m_default_policy_params[current_bag], context));
			}

			if (action_from_bag.Get_Id() == 0 || action_from_bag.Get_Id() > actions.Count())
			{
				throw std::invalid_argument("Action chosen by default policy is not within valid range.");
			}

			if (current_bag == chosen_bag)
			{
				chosen_action = action_from_bag;
			}
			//this won't work if actions aren't 0 to Count
			actions_selected[action_from_bag.Get_Id_ZeroBased()]++;
		}
		float action_probability = (float)actions_selected[chosen_action.Get_Id_ZeroBased()] / m_bags;

		return std::tuple<MWTAction, float, bool>(chosen_action, action_probability, true);
	}

private:
	u32 m_bags;
	u64 m_salt;

	Stateful_Policy_Func** m_stateful_default_policy_funcs;
	Stateless_Policy_Func** m_stateless_default_policy_funcs;
	void** m_default_policy_params;
};
}

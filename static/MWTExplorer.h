//
// Main interface for clients of the Multiworld testing (MWT) service.
//

#pragma once

#include <stdexcept>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <utility>
#include <memory>
#include <limits.h>
#include <tuple>

#ifdef MANAGED_CODE
#define PORTING_INTERFACE public
#define MWT_NAMESPACE namespace NativeMultiWorldTesting
#else
#define PORTING_INTERFACE private
#define MWT_NAMESPACE namespace MultiWorldTesting
#endif

using namespace std;

#include "utility.h"

/** \defgroup MultiWorldTestingCpp
\brief C++ implementation, for sample usage see: https://github.com/sidsen/vowpal_wabbit/blob/v0/explore/explore_sample.cpp
*/

/*!
*  \addtogroup MultiWorldTestingCpp
*  @{
*/

//! Interface for C++ version of Multiworld Testing library.
//! For sample usage see: https://github.com/sidsen/vowpal_wabbit/blob/v0/explore/explore_sample.cpp
MWT_NAMESPACE {

// Forward declarations
template <class Ctx> 
class IRecorder;
template <class Ctx>
class IExplorer;

///
/// The top-level MwtExplorer class. Using this enables principled and efficient exploration
/// over a set of possible actions, and ensures that the right bits are recorded.
///
template <class Ctx>
class MwtExplorer
{
public:
	///
	/// Constructor
	///
	/// @param appid      This should be unique to your experiment or you risk nasty correlation bugs.
	/// @param recorder   A user-specified class for recording the appropriate bits for use in evaluation and learning.
	///
	MwtExplorer(std::string app_id, IRecorder<Ctx>& recorder) : m_recorder(recorder)
	{
		m_app_id = HashUtils::Compute_Id_Hash(app_id);
	}

	///
	/// Chooses an action by invoking an underlying exploration algorithm. This should be a 
	/// drop-in replacement for any existing policy function.   
	///
	/// @param explorer    An existing exploration algorithm (one of the below) which uses the default policy as a callback.
	/// @param unique_key  A unique identifier for the experimental unit. This could be a user id, a session id, etc..
	/// @param context     The context upon which a decision is made. See SimpleContext below for an example.
	///
	u32 Choose_Action(IExplorer<Ctx>& explorer, string unique_key, Ctx& context)
	{
		u64 seed = HashUtils::Compute_Id_Hash(unique_key);

		std::tuple<u32, float, bool> action_probability_log_tuple = explorer.Choose_Action(seed + m_app_id, context);

		u32 action = std::get<0>(action_probability_log_tuple);
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

///
/// Exposes a method to record exploration data based on generic contexts. Exploration data
/// is specified as a set of tuples <context, action, probability, key> as described below. An 
/// application passes an IRecorder object to the @MwtExplorer constructor. See 
/// @StringRecorder for a sample IRecorder object.
///
template <class Ctx>
class IRecorder
{
public:
	///
	/// Records the exploration data associated with a given decision.
	///
	/// @param context      A user-defined context for the decision
	/// @param action       The action chosen by an exploration algorithm given context
	/// @param probability  The probability the exploration algorithm chose said action 
	/// @param unique_key   A user-defined unique identifer for the decision
	///
	virtual void Record(Ctx& context, u32 action, float probability, string unique_key) = 0;
    virtual ~IRecorder() { }
};

///
/// Exposes a method to choose an action given a generic context, and obtain the relevant
/// exploration bits. Invokes IPolicy::Choose_Action internally. Do not implement this 
/// interface yourself: instead, use the various exploration algorithms below, which 
/// implement it for you. 
///
template <class Ctx>
class IExplorer
{
public:
	///
	/// Determines the action to take and the probability with which it was chosen, for a
	/// given context.
	///
	/// @param salted_seed  A PRG seed based on a unique id information provided by the user
	/// @param context      A user-defined context for the decision
	/// @returns            The action to take, the probability it was chosen, and a flag indicating 
	///                     whether to record this decision
	///
	virtual std::tuple<u32, float, bool> Choose_Action(u64 salted_seed, Ctx& context) = 0;
    virtual void Enable_Explore(bool explore) = 0;
    virtual ~IExplorer() { }
};

///
/// Exposes a method to choose an action given a generic context. IPolicy objects are 
/// passed to (and invoked by) exploration algorithms to specify the default policy behavior.
///
template <class Ctx>
class IPolicy
{
public:
	///
	/// Determines the action to take for a given context.
	///
	/// @param context   A user-defined context for the decision
	/// @returns	        The action to take (1-based index)
	///
	virtual u32 Choose_Action(Ctx& context) = 0;
    virtual ~IPolicy() { }
};

///
/// Exposes a method for specifying a score (weight) for each action given a generic context. 
///
template <class Ctx>
class IScorer
{
public:
	///
	/// Determines the score of each action for a given context.
	///
	/// @param context   A user-defined context for the decision 
	/// @returns         A vector of scores indexed by action (1-based)
	///
	virtual vector<float> Score_Actions(Ctx& context) = 0;
    virtual ~IScorer() { }
};

template <class Ctx>
class IConsumePolicy
{
public:
    virtual void Update_Policy(IPolicy<Ctx>& new_policy) = 0;
    virtual ~IConsumePolicy() { }
};

template <class Ctx>
class IConsumePolicies
{
public:
    virtual void Update_Policy(vector<unique_ptr<IPolicy<Ctx>>>& new_policy_functions) = 0;
    virtual ~IConsumePolicies() { }
};

template <class Ctx>
class IConsumeScorer
{
public:
    virtual void Update_Scorer(IScorer<Ctx>& new_policy) = 0;
    virtual ~IConsumeScorer() { }
};

///
/// A sample recorder class that converts the exploration tuple into string format.
///
template <class Ctx>
struct StringRecorder : public IRecorder<Ctx>
{
	void Record(Ctx& context, u32 action, float probability, string unique_key)
	{
		// Implicitly enforce To_String() API on the context
	  m_recording.append(to_string((unsigned long long)action));
		m_recording.append(" ", 1);
		m_recording.append(unique_key);
		m_recording.append(" ", 1);

		char prob_str[10] = { 0 };
        int x = (int)probability;
        int d = (int)(fabs(probability - x) * 100000);
        sprintf_s(prob_str, 10 * sizeof(char), "%d.%05d", x, d);
		m_recording.append(prob_str);

		m_recording.append(" | ", 3);
		m_recording.append(context.To_String());
		m_recording.append("\n");
	}

	// Gets the content of the recording so far as a string and optionally clears internal content.
	string Get_Recording(bool flush = true)
	{
		if (!flush)
		{
			return m_recording;
		}
		string recording = m_recording;
		m_recording.clear();
		return recording;
	}

private:
	string m_recording;
};

///
/// Represents a feature in a sparse array.
///
struct Feature
{
	float Value;
	u32 Id;

	bool operator==(Feature other_feature)
	{
		return Id == other_feature.Id;
	}
};

///
/// A sample context class that stores a vector of Features.
///
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
		const size_t strlen = 35;
		char feature_str[strlen] = { 0 };
		for (size_t i = 0; i < m_features.size(); i++)
		{
			int chars;
			if (i == 0)
			{
                chars = sprintf_s(feature_str, strlen, "%d:", m_features[i].Id);
			}
			else
			{
                chars = sprintf_s(feature_str, strlen, " %d:", m_features[i].Id);
			}
            NumberUtils::print_float(feature_str + chars, strlen-chars, m_features[i].Value);
			out_string.append(feature_str);
		}
		return out_string;
	}

private:
	vector<Feature>& m_features;
};

///
/// The epsilon greedy exploration algorithm. This is a good choice if you have no idea 
/// which actions should be preferred.  Epsilon greedy is also computationally cheap.
///
template <class Ctx>
class EpsilonGreedyExplorer : public IExplorer<Ctx>, public IConsumePolicy<Ctx>
{
public:
	///
	/// The constructor is the only public member, because this should be used with the MwtExplorer.
	///
	/// @param default_policy  A default function which outputs an action given a context.
	/// @param epsilon         The probability of a random exploration.
	/// @param num_actions     The number of actions to randomize over.
	///
	EpsilonGreedyExplorer(IPolicy<Ctx>& default_policy, float epsilon, u32 num_actions) :
        m_default_policy(default_policy), m_epsilon(epsilon), m_num_actions(num_actions), m_explore(true)
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

    void Update_Policy(IPolicy<Ctx>& new_policy)
    {
        m_default_policy = new_policy;
    }

    void Enable_Explore(bool explore)
    {
        m_explore = explore;
    }

private:
	std::tuple<u32, float, bool> Choose_Action(u64 salted_seed, Ctx& context)
	{
		PRG::prg random_generator(salted_seed);

		// Invoke the default policy function to get the action
		u32 chosen_action = m_default_policy.Choose_Action(context);

		if (chosen_action == 0 || chosen_action > m_num_actions)
		{
			throw std::invalid_argument("Action chosen by default policy is not within valid range.");
		}

        float epsilon = m_explore ? m_epsilon : 0.f;

		float action_probability = 0.f;
        float base_probability = epsilon / m_num_actions; // uniform probability

		// TODO: check this random generation
        if (random_generator.Uniform_Unit_Interval() < 1.f - epsilon)
		{
            action_probability = 1.f - epsilon + base_probability;
		}
		else
		{
			// Get uniform random action ID
			u32 actionId = random_generator.Uniform_Int(1, m_num_actions);

			if (actionId == chosen_action)
			{
				// IF it matches the one chosen by the default policy
				// then increase the probability
                action_probability = 1.f - epsilon + base_probability;
			}
			else
			{
				// Otherwise it's just the uniform probability
				action_probability = base_probability;
			}
			chosen_action = actionId;
		}

		return std::tuple<u32, float, bool>(chosen_action, action_probability, true);
	}

private:
	IPolicy<Ctx>& m_default_policy;
	float m_epsilon;
    bool m_explore;
	u32 m_num_actions;

private:
	friend class MwtExplorer<Ctx>;
};

///
/// In some cases, different actions have a different scores, and you would prefer to
/// choose actions with large scores. Softmax allows you to do that.
/// 
template <class Ctx>
class SoftmaxExplorer : public IExplorer<Ctx>, public IConsumeScorer<Ctx>
{
public:
	///
    /// The constructor is the only public member, because this should be used with the MwtExplorer.
    ///
    /// @param default_scorer  A function which outputs a score for each action.
    /// @param lambda          lambda = 0 implies uniform distribution.  Large lambda is equivalent to a max.
    /// @param num_actions     The number of actions to randomize over.
	///
	SoftmaxExplorer(IScorer<Ctx>& default_scorer, float lambda, u32 num_actions) :
        m_default_scorer(default_scorer), m_lambda(lambda), m_num_actions(num_actions), m_explore(true)
	{
		if (m_num_actions < 1)
		{
			throw std::invalid_argument("Number of actions must be at least 1.");
		}
	}

    void Update_Scorer(IScorer<Ctx>& new_scorer)
    {
        m_default_scorer = new_scorer;
    }

    void Enable_Explore(bool explore)
    {
        m_explore = explore;
    }

private:
	std::tuple<u32, float, bool> Choose_Action(u64 salted_seed, Ctx& context)
	{
		PRG::prg random_generator(salted_seed);

		// Invoke the default scorer function
		vector<float> scores = m_default_scorer.Score_Actions(context);
		u32 num_scores = (u32)scores.size();
		if (num_scores != m_num_actions)
		{
			throw std::invalid_argument("The number of scores returned by the scorer must equal number of actions");
		}

		u32 i = 0;

		float max_score = -FLT_MAX;
		for (i = 0; i < num_scores; i++)
		{
			if (max_score < scores[i])
			{
				max_score = scores[i];
			}
		}

        float action_probability = 0.f;
        u32 action_index = 0;
        if (m_explore)
        {
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
            action_probability = 0.f;
            action_index = num_scores - 1;
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
        }
        else
        {
            float max_score = 0.f;
            for (size_t i = 0; i < num_scores; i++)
            {
                if (max_score < scores[i])
                {
                    max_score = scores[i];
                    action_index = (u32)i;
                }
            }
            action_probability = 1.f; // Set to 1 since we always pick the highest one.
        }

		// action id is one-based
		return std::tuple<u32, float, bool>(action_index + 1, action_probability, true);
	}

private:
	IScorer<Ctx>& m_default_scorer;
    bool m_explore;
	float m_lambda;
	u32 m_num_actions;

private:
	friend class MwtExplorer<Ctx>;
};

///
/// GenericExplorer provides complete flexibility.  You can create any
/// distribution over actions desired, and it will draw from that.
/// 
template <class Ctx>
class GenericExplorer : public IExplorer<Ctx>, public IConsumeScorer<Ctx>
{
public:
	///
	/// The constructor is the only public member, because this should be used with the MwtExplorer.
	///
    /// @param default_scorer  A function which outputs the probability of each action.
    /// @param num_actions     The number of actions to randomize over.
	///
	GenericExplorer(IScorer<Ctx>& default_scorer, u32 num_actions) :
        m_default_scorer(default_scorer), m_num_actions(num_actions), m_explore(true)
	{
		if (m_num_actions < 1)
		{
			throw std::invalid_argument("Number of actions must be at least 1.");
		}
	}

    void Update_Scorer(IScorer<Ctx>& new_scorer)
    {
        m_default_scorer = new_scorer;
    }

    void Enable_Explore(bool explore)
    {
        m_explore = explore;
    }

private:
	std::tuple<u32, float, bool> Choose_Action(u64 salted_seed, Ctx& context)
	{
		PRG::prg random_generator(salted_seed);

		// Invoke the default scorer function
		vector<float> weights = m_default_scorer.Score_Actions(context);
		u32 num_weights = (u32)weights.size();
		if (num_weights != m_num_actions)
		{
			throw std::invalid_argument("The number of weights returned by the scorer must equal number of actions");
		}

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

		// action id is one-based
		return std::tuple<u32, float, bool>(action_index + 1, action_probability, true);
	}

private:
	IScorer<Ctx>& m_default_scorer;
    bool m_explore;
	u32 m_num_actions;

private:
	friend class MwtExplorer<Ctx>;
};

///
/// The tau-first explorer collects exactly tau uniform random exploration events, and then 
/// uses the default policy thereafter.
/// 
template <class Ctx>
class TauFirstExplorer : public IExplorer<Ctx>, public IConsumePolicy<Ctx>
{
public:

	///
	/// The constructor is the only public member, because this should be used with the MwtExplorer.
	///
    /// @param default_policy  A default policy after randomization finishes.
    /// @param tau             The number of events to be uniform over.
    /// @param num_actions     The number of actions to randomize over.
	///
	TauFirstExplorer(IPolicy<Ctx>& default_policy, u32 tau, u32 num_actions) :
        m_default_policy(default_policy), m_tau(tau), m_num_actions(num_actions), m_explore(true)
	{
		if (m_num_actions < 1)
		{
			throw std::invalid_argument("Number of actions must be at least 1.");
		}
	}

    void Update_Policy(IPolicy<Ctx>& new_policy)
    {
        m_default_policy = new_policy;
    }

    void Enable_Explore(bool explore)
    {
        m_explore = explore;
    }

private:
	std::tuple<u32, float, bool> Choose_Action(u64 salted_seed, Ctx& context)
	{
		PRG::prg random_generator(salted_seed);

		u32 chosen_action = 0;
		float action_probability = 0.f;
		bool log_action;

        if (m_tau && m_explore)
		{
            m_tau--;
			u32 actionId = random_generator.Uniform_Int(1, m_num_actions);
			action_probability = 1.f / m_num_actions;
			chosen_action = actionId;
			log_action = true;
		}
		else
		{
			// Invoke the default policy function to get the action
			chosen_action = m_default_policy.Choose_Action(context);

			if (chosen_action == 0 || chosen_action > m_num_actions)
			{
				throw std::invalid_argument("Action chosen by default policy is not within valid range.");
			}

			action_probability = 1.f;
			log_action = false;
		}

		return std::tuple<u32, float, bool>(chosen_action, action_probability, log_action);
	}

private:
	IPolicy<Ctx>& m_default_policy;
    bool m_explore;
	u32 m_tau;
	u32 m_num_actions;

private:
	friend class MwtExplorer<Ctx>;
};

///
/// The Bootstrap explorer randomizes over the actions chosen by a set of default policies. 
/// This performs well statistically but can be computationally expensive.
/// 
template <class Ctx>
class BootstrapExplorer : public IExplorer<Ctx>, public IConsumePolicies<Ctx>
{
public:
	///
	/// The constructor is the only public member, because this should be used with the MwtExplorer.
    ///
    /// @param default_policy_functions  A set of default policies to be uniform random over. 
	/// The policy pointers must be valid throughout the lifetime of this explorer.
    /// @param num_actions               The number of actions to randomize over.
	///
	BootstrapExplorer(vector<unique_ptr<IPolicy<Ctx>>>& default_policy_functions, u32 num_actions) :
		m_default_policy_functions(default_policy_functions),
        m_num_actions(num_actions), m_explore(true)
	{
	        m_bags = (u32)default_policy_functions.size();
		if (m_num_actions < 1)
		{
			throw std::invalid_argument("Number of actions must be at least 1.");
		}

		if (m_bags < 1)
		{
			throw std::invalid_argument("Number of bags must be at least 1.");
		}
	}

    void Update_Policy(vector<unique_ptr<IPolicy<Ctx>>>& new_policy_functions)
    {
        m_default_policy_functions = move(new_policy_functions);
    }

    void Enable_Explore(bool explore)
    {
        m_explore = explore;
    }

private:
	std::tuple<u32, float, bool> Choose_Action(u64 salted_seed, Ctx& context)
	{
		PRG::prg random_generator(salted_seed);

		// Select bag
        u32 chosen_bag = random_generator.Uniform_Int(0, m_bags - 1);

		// Invoke the default policy function to get the action
		u32 chosen_action = 0;
        float action_probability = 0.f;

        if (m_explore)
        {
            u32 action_from_bag = 0;
            vector<u32> actions_selected;
            for (size_t i = 0; i < m_num_actions; i++)
            {
                actions_selected.push_back(0);
            }

            // Invoke the default policy function to get the action
            for (u32 current_bag = 0; current_bag < m_bags; current_bag++)
            {
                action_from_bag = m_default_policy_functions[current_bag]->Choose_Action(context);

                if (action_from_bag == 0 || action_from_bag > m_num_actions)
                {
                    throw std::invalid_argument("Action chosen by default policy is not within valid range.");
                }

                if (current_bag == chosen_bag)
                {
                    chosen_action = action_from_bag;
                }
                //this won't work if actions aren't 0 to Count
                actions_selected[action_from_bag - 1]++; // action id is one-based
            }
            action_probability = (float)actions_selected[chosen_action - 1] / m_bags; // action id is one-based
        }
        else
        {
            chosen_action = m_default_policy_functions[0]->Choose_Action(context);
            action_probability = 1.f;
        }

		return std::tuple<u32, float, bool>(chosen_action, action_probability, true);
	}

private:
	vector<unique_ptr<IPolicy<Ctx>>>& m_default_policy_functions;
    bool m_explore;
    u32 m_bags;
	u32 m_num_actions;

private:
	friend class MwtExplorer<Ctx>;
};
} // End namespace MultiWorldTestingCpp
/*! @} End of Doxygen Groups*/

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
#include <Windows.h>

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
    void Choose_Action(IExplorer<Ctx>& explorer, string unique_key, Ctx& context, u32* actions, u32 num_actions)
    {
        u64 seed = HashUtils::Compute_Id_Hash(unique_key);

        std::tuple<float, bool> action_probability_log_tuple = explorer.Choose_Action(seed + m_app_id, context, actions, num_actions);

        float prob = std::get<0>(action_probability_log_tuple);

        if (std::get<1>(action_probability_log_tuple))
        {
            m_recorder.Record(context, actions, num_actions, prob, unique_key);
        }
    }

PORTING_INTERFACE:
    u32 Get_Number_Of_Actions(IExplorer<Ctx>& explorer, Ctx& context)
    {
        return explorer.Get_Number_Of_Actions(context);
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
    /// This implementation should be thread-safe if multithreading is needed.
    ///
    /// @param context      A user-defined context for the decision
    /// @param action       The action chosen by an exploration algorithm given context
    /// @param probability  The probability the exploration algorithm chose said action 
    /// @param unique_key   A user-defined unique identifer for the decision
    ///
    virtual void Record(Ctx& context, u32* actions, u32 num_actions, float probability, string unique_key) = 0;
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
    virtual std::tuple<float, bool> Choose_Action(u64 salted_seed, Ctx& context, u32* actions, u32 num_actions) = 0;
    virtual void Enable_Explore(bool explore) = 0;
    virtual ~IExplorer() { }

PORTING_INTERFACE:
    virtual u32 Get_Number_Of_Actions(Ctx& context) = 0;
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
    /// This implementation should be thread-safe if multithreading is needed.
    ///
    /// @param context   A user-defined context for the decision
    /// @returns	        The action to take (1-based index)
    ///
    virtual void Choose_Action(Ctx& context, u32* actions, u32 num_actions) = 0;
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
    /// This implementation should be thread-safe if multithreading is needed.
    ///
    /// @param context   A user-defined context for the decision 
    /// @returns         A vector of scores indexed by action (1-based)
    ///
    virtual vector<float> Score_Actions(Ctx& context) = 0;
    virtual ~IScorer() { }
};

///
/// Represents a context interface with variable number of actions which is
/// enforced if exploration algorithm is initialized in variable number of actions mode.
///
class IVariableActionContext
{
public:
    ///
    /// Gets the number of actions for the current context.
    ///
    /// @returns            The number of actions available for the current context.
    ///
    virtual u32 Get_Number_Of_Actions() = 0;
    virtual ~IVariableActionContext() { }
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
    void Record(Ctx& context, u32* actions, u32 num_actions, float probability, string unique_key)
    {
        // Implicitly enforce To_String() API on the context
        m_recording.append(to_string((unsigned long long)actions[0])); // TODO: serialize the whole list of actions instead of just the top 1
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

template <class Ctx>
static u32 Get_Variable_Number_Of_Actions(Ctx& context, u32 default_num_actions)
{
    u32 num_actions = default_num_actions;
    if (num_actions == UINT_MAX)
    {
        num_actions = ((IVariableActionContext*)(&context))->Get_Number_Of_Actions();
        if (num_actions < 1)
        {
            throw std::invalid_argument("Number of actions must be at least 1.");
        }
    }
    return num_actions;
}

static void Sample_Without_Replacement(u32* actions, vector<float>& probs, u32 size, PRG::prg& random_generator, float& top_action_probability)
{
    for (u32 i = 0; i < size; i++)
    {
        if (probs[i] == 1.f)
        {
            throw std::invalid_argument("The resulting probability distribution is deterministic and thus cannot generate a list of unique actions.");
        }
    }

    // sample without replacement
    u32 running_index = 0;
    u32 running_action = 0;
    float draw, sum;
    while (running_index < size)
    {
        draw = random_generator.Uniform_Unit_Interval();
        sum = 0.f;

        for (u32 i = 0; i < size; i++)
        {
            sum += probs[i];
            if (sum > draw)
            {
                running_action = (u32)(i + 1);

                // TODO: make this more efficient
                // check for duplicate
                bool exists = false;
                for (u32 j = 0; j <= running_index; j++)
                {
                    if (actions[j] == running_action)
                    {
                        exists = true;
                        break;
                    }
                }
                if (exists)
                {
                    continue;
                }

                // store newly sampled action
                if (running_index == 0)
                {
                    top_action_probability = probs[i];
                }
                actions[running_index++] = running_action;
                break;
            }
        }
    }
}

void Validate_Actions(u32* actions, u32 num_actions)
{
    unique_ptr<bool> exists_ptr(new bool[num_actions + 1]());
    
    bool* exists = exists_ptr.get();
    for (u32 i = 0; i < num_actions; i++)
    {
        if (actions[i] == 0 || actions[i] > num_actions)
        {
            throw std::invalid_argument("Action chosen by default policy is not within valid range.");
        }
        if (exists[actions[i]])
        {
            throw std::invalid_argument("List of actions cannot contain duplicates.");
        }
        exists[actions[i]] = true;
    }
};

void Put_Action_To_List(u32 action, u32* actions, u32 num_actions)
{
    for (u32 i = 0; i < num_actions; i++)
    {
        if (action == actions[i])
        {
            // swap
            actions[i] = actions[0];
            actions[0] = action;

            return;
        }
    }

    // Specified action isn't in the list, just replace the top. 
    actions[0] = action;
}

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

    ///
    /// Initializes an epsilon greedy explorer with variable number of actions.
    ///
    /// @param default_policy  A default function which outputs an action given a context.
    /// @param epsilon         The probability of a random exploration.
    ///
    EpsilonGreedyExplorer(IPolicy<Ctx>& default_policy, float epsilon) :
        m_default_policy(default_policy), m_epsilon(epsilon), m_num_actions(UINT_MAX), m_explore(true)
    {
        if (m_epsilon < 0 || m_epsilon > 1)
        {
            throw std::invalid_argument("Epsilon must be between 0 and 1.");
        }
        static_assert(std::is_base_of<IVariableActionContext, Ctx>::value, "The provided context does not implement variable-action interface.");
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
    std::tuple<float, bool> Choose_Action(u64 salted_seed, Ctx& context, u32* actions, u32 num_actions)
    {
        PRG::prg random_generator(salted_seed);

        // Invoke the default policy function to get the action
        m_default_policy.Choose_Action(context, actions, num_actions);
        ::Validate_Actions(actions, num_actions);

        float epsilon = m_explore ? m_epsilon : 0.f;

        float action_probability = 0.f;
        float base_probability = epsilon / num_actions; // uniform probability

        // TODO: check this random generation
        if (random_generator.Uniform_Unit_Interval() < 1.f - epsilon)
        {
            action_probability = 1.f - epsilon + base_probability;
        }
        else
        {
            // Get uniform random action ID
            u32 actionId = random_generator.Uniform_Int(1, num_actions);

            if (actionId == actions[0])
            {
                // IF it matches the one chosen by the default policy
                // then increase the probability
                action_probability = 1.f - epsilon + base_probability;
            }
            else
            {
                // Otherwise it's just the uniform probability
                action_probability = base_probability;

                ::Put_Action_To_List(actionId, actions, num_actions);
            }
        }

        return std::tuple<float, bool>(action_probability, true);
    }

PORTING_INTERFACE:

    u32 Get_Number_Of_Actions(Ctx& context)
    {
        return ::Get_Variable_Number_Of_Actions(context, m_num_actions);
    }

private:
    IPolicy<Ctx>& m_default_policy;
    const float m_epsilon;
    bool m_explore;
    const u32 m_num_actions;
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

    ///
    /// Initializes a softmax explorer with variable number of actions.
    ///
    /// @param default_scorer  A function which outputs a score for each action.
    /// @param lambda          lambda = 0 implies uniform distribution.  Large lambda is equivalent to a max.
    ///
    SoftmaxExplorer(IScorer<Ctx>& default_scorer, float lambda) :
        m_default_scorer(default_scorer), m_lambda(lambda), m_num_actions(UINT_MAX), m_explore(true)
    {
        static_assert(std::is_base_of<IVariableActionContext, Ctx>::value, "The provided context does not implement variable-action interface.");
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
    std::tuple<float, bool> Choose_Action(u64 salted_seed, Ctx& context, u32* actions, u32 num_actions)
    {
        PRG::prg random_generator(salted_seed);

        // Invoke the default scorer function
        vector<float> scores = m_default_scorer.Score_Actions(context);
        u32 num_scores = (u32)scores.size();
        if (num_scores != num_actions)
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

            // normalize scores & reset actions
            for (size_t i = 0; i < num_scores; i++)
            {
                scores[i] = scores[i] / total;
                actions[i] = 0;
            }

            ::Sample_Without_Replacement(actions, scores, num_actions, random_generator, action_probability);
        }
        else
        {
            // prefill list of actions
            for (size_t i = 0; i < num_actions; i++)
            {
                actions[i] = (u32)(i + 1);
            }

            // find action with the highest score
            u32 max_index = 0;
            float max_score = 0.f;
            for (u32 i = 0; i < num_scores; i++)
            {
                if (max_score < scores[i])
                {
                    max_score = scores[i];
                    max_index = i;
                }
            }
            // swap max-score action with the first one
            u32 first_action = actions[0];
            actions[0] = actions[max_index];
            actions[max_index] = first_action;

            action_probability = 1.f; // Set to 1 since we always pick the highest one.
        }

        return std::tuple<float, bool>(action_probability, true);
    }

PORTING_INTERFACE:

    u32 Get_Number_Of_Actions(Ctx& context)
    {
        return ::Get_Variable_Number_Of_Actions(context, m_num_actions);
    }

private:
    IScorer<Ctx>& m_default_scorer;
    bool m_explore;
    const float m_lambda;
    const u32 m_num_actions;
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

    ///
    /// Initializes a generic explorer with variable number of actions.
    ///
    /// @param default_scorer  A function which outputs the probability of each action.
    ///
    GenericExplorer(IScorer<Ctx>& default_scorer) :
        m_default_scorer(default_scorer), m_num_actions(UINT_MAX), m_explore(true)
    {
        static_assert(std::is_base_of<IVariableActionContext, Ctx>::value, "The provided context does not implement variable-action interface.");
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
    std::tuple<float, bool> Choose_Action(u64 salted_seed, Ctx& context, u32* actions, u32 num_actions)
    {
        PRG::prg random_generator(salted_seed);

        // Invoke the default scorer function
        vector<float> weights = m_default_scorer.Score_Actions(context);
        u32 num_weights = (u32)weights.size();
        if (num_weights != num_actions)
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

        // normalize weights & reset actions
        for (size_t i = 0; i < num_weights; i++)
        {
            weights[i] = weights[i] / total;
            actions[i] = 0;
        }

        float action_probability = 0.f;
        ::Sample_Without_Replacement(actions, weights, num_actions, random_generator, action_probability);

        return std::tuple<float, bool>(action_probability, true);
    }

PORTING_INTERFACE:

    u32 Get_Number_Of_Actions(Ctx& context)
    {
        return ::Get_Variable_Number_Of_Actions(context, m_num_actions);
    }

private:
    IScorer<Ctx>& m_default_scorer;
    bool m_explore;
    const u32 m_num_actions;
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

        ::InitializeCriticalSection(&m_critical_section);
    }

    ///
    /// Initializes a tau-first explorer with variable number of actions.
    ///
    /// @param default_policy  A default policy after randomization finishes.
    /// @param tau             The number of events to be uniform over.
    ///
    TauFirstExplorer(IPolicy<Ctx>& default_policy, u32 tau) :
        m_default_policy(default_policy), m_tau(tau), m_num_actions(UINT_MAX), m_explore(true)
    {
        static_assert(std::is_base_of<IVariableActionContext, Ctx>::value, "The provided context does not implement variable-action interface.");

        ::InitializeCriticalSection(&m_critical_section);
    }

    ~TauFirstExplorer() 
    {
        ::DeleteCriticalSection(&m_critical_section);
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
    std::tuple<float, bool> Choose_Action(u64 salted_seed, Ctx& context, u32* actions, u32 num_actions)
    {
        PRG::prg random_generator(salted_seed);

        float action_probability = 0.f;
        bool log_action;

        // Invoke the default policy function to get the action
        m_default_policy.Choose_Action(context, actions, num_actions);
        ::Validate_Actions(actions, num_actions);
        
        bool explore = false;
        if (m_explore)
        {
            // TODO: add non-windows locking mechanism
            ::EnterCriticalSection(&m_critical_section);

            if (m_tau)
            {
                m_tau--;
                explore = true;
            }

            ::LeaveCriticalSection(&m_critical_section);
        }

        if (explore)
        {
            u32 actionId = random_generator.Uniform_Int(1, num_actions);
            action_probability = 1.f / num_actions;

            ::Put_Action_To_List(actionId, actions, num_actions);
            
            log_action = true;
        }
        else
        {
            action_probability = 1.f;
            log_action = false;
        }

        return std::tuple<float, bool>(action_probability, log_action);
    }

PORTING_INTERFACE:

    u32 Get_Number_Of_Actions(Ctx& context)
    {
        return ::Get_Variable_Number_Of_Actions(context, m_num_actions);
    }

private:
    IPolicy<Ctx>& m_default_policy;
    bool m_explore;
    u32 m_tau;
    const u32 m_num_actions;
    CRITICAL_SECTION m_critical_section;
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
        m_num_actions(num_actions), m_explore(true), m_bags((u32)default_policy_functions.size())
    {
        if (m_num_actions < 1)
        {
            throw std::invalid_argument("Number of actions must be at least 1.");
        }

        if (m_bags < 1)
        {
            throw std::invalid_argument("Number of bags must be at least 1.");
        }
    }

    ///
    /// Initializes a bootstrap explorer with variable number of actions.
    ///
    /// @param default_policy_functions  A set of default policies to be uniform random over. 
    /// The policy pointers must be valid throughout the lifetime of this explorer.
    ///
    BootstrapExplorer(vector<unique_ptr<IPolicy<Ctx>>>& default_policy_functions) :
        m_default_policy_functions(default_policy_functions),
        m_num_actions(UINT_MAX), m_explore(true), m_bags((u32)default_policy_functions.size())
    {
        if (m_bags < 1)
        {
            throw std::invalid_argument("Number of bags must be at least 1.");
        }

        static_assert(std::is_base_of<IVariableActionContext, Ctx>::value, "The provided context does not implement variable-action interface.");
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
    std::tuple<float, bool> Choose_Action(u64 salted_seed, Ctx& context, u32* actions, u32 num_actions)
    {
        PRG::prg random_generator(salted_seed);

        // Select bag
        u32 chosen_bag = random_generator.Uniform_Int(0, m_bags - 1);

        // Invoke the default policy function to get the action
        u32 chosen_top_action = 0;
        float action_probability = 0.f;

        if (m_explore)
        {
            u32 top_action_from_bag = 0;
            vector<u32> actions_selected;
            for (size_t i = 0; i < num_actions; i++)
            {
                actions_selected.push_back(0);
            }

            unique_ptr<u32> chosen_actions_ptr(new u32[num_actions]);
            u32* chosen_actions = chosen_actions_ptr.get();

            // Invoke the default policy function to get the action
            for (u32 current_bag = 0; current_bag < m_bags; current_bag++)
            {
                // TODO: can VW predict for all bags on one call? (returning all actions at once)
                // if we trigger into VW passing an index to invoke bootstrap scoring, and if VW model changes while we are doing so, 
                // we could end up calling the wrong bag
                m_default_policy_functions[current_bag]->Choose_Action(context, actions, num_actions);
                ::Validate_Actions(actions, num_actions);

                top_action_from_bag = actions[0];

                if (current_bag == chosen_bag)
                {
                    // store the list of actions chosen by the selected bag
                    // this is needed since 'actions' array is pre-allocated by managed code
                    // and will be overidden by the next policy::choose_action
                    ::memcpy(chosen_actions, actions, num_actions * sizeof(u32));
                    chosen_top_action = top_action_from_bag;
                }
                //this won't work if actions aren't 0 to Count
                actions_selected[top_action_from_bag - 1]++; // action id is one-based
            }
            action_probability = (float)actions_selected[chosen_top_action - 1] / m_bags; // action id is one-based
            
            // restore the list of actions chosen by the selected bag
            ::memcpy(actions, chosen_actions, num_actions * sizeof(u32));
        }
        else
        {
            m_default_policy_functions[0]->Choose_Action(context, actions, num_actions);
            action_probability = 1.f;
        }

        return std::tuple<float, bool>(action_probability, true);
    }

PORTING_INTERFACE:

    u32 Get_Number_Of_Actions(Ctx& context)
    {
        return ::Get_Variable_Number_Of_Actions(context, m_num_actions);
    }

private:
    vector<unique_ptr<IPolicy<Ctx>>>& m_default_policy_functions;
    bool m_explore;
    const u32 m_bags;
    const u32 m_num_actions;
};

} // End namespace MultiWorldTestingCpp
/*! @} End of Doxygen Groups*/

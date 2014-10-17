//
// Classes and definitions for interacting with the MWT service.
//
#pragma once

#include <iomanip>
#include <iostream>
#include <sstream>
#include <float.h>
#include "Common.h" 

using namespace std;

//TODO: Make this a static const float of the Interaction class (right now VS doesn't support
// the C++11 constexpr keyword, which is needed to initialize such non-integer types.
#define NO_REWARD -FLT_MAX

class Serializable
{
public:
	virtual void Serialize(std::ostringstream&) = 0;
};

struct MWTFeature
{
	float X;
	u32 Index;

	bool operator==(MWTFeature other_feature)
	{
		return Index == other_feature.Index;
	}
};

class MWTAction : public Serializable
{
public:
	MWTAction(u32 id) : m_id(id)
	{
	}
	u32 Get_Id() const { return m_id; }
	u32 Get_Id_ZeroBased() const { return m_id - 1; }

	static u32 Make_OneBased(u32 id) { return id + 1; }
	static u32 Make_ZeroBased(u32 id) { return id - 1; }

	//TODO: Consider making this virtual and extensible (depends on ActionSet and what we expose to the user)
	bool Match(MWTAction& second_action)
	{
		return m_id == second_action.Get_Id();
	}

	void Serialize(std::ostringstream& stream)
	{
		stream << m_id;
	}

private:
	u32 m_id;
};

class ActionSet
{
public:

	// TODO: support opaque IDs, will need to update Action class as well
	// e.g. add GetStringID() or GetDescription() etc...
	ActionSet() : m_count(0)
	{
	}

	~ActionSet()
	{
	}

	MWTAction Get(u32 id)
	{
		return MWTAction(id);
	}

	void Set_Count(u32 num_actions)
	{
		m_count = num_actions;
	}

	u32 Count()
	{
		return m_count;
	}

private:
	u32 m_count;
};

class Context : public Serializable
{
public:
	Context(MWTFeature* common_features, size_t num_features, bool is_copy = false) : 
		m_common_features(common_features), 
		m_num_features(num_features),
		m_other_context(nullptr),
		m_is_copy(is_copy)
	{
	}

	Context(MWTFeature* common_features, size_t num_features,
		std::string* other_context, bool is_copy = false) :
		m_common_features(common_features), 
		m_num_features(num_features), 
		m_other_context(other_context),
		m_is_copy(is_copy)
	{
	}

	Context(MWTFeature* common_features, size_t num_features,
		std::string external_context, bool is_copy = false) :
		m_common_features(common_features),
		m_num_features(num_features),
		m_external_other_context(external_context),
		m_is_copy(is_copy)
	{
		m_other_context = &external_context;
	}

	~Context()
	{
		if (m_is_copy)
		{
			delete[] m_common_features;
			delete m_other_context;
		}
	}

	Context* Copy()
	{
		MWTFeature* features = nullptr;
		std::string* other_context = nullptr;

		if (m_num_features > 0 && m_common_features != nullptr)
		{
			features = new MWTFeature[m_num_features];
			for (size_t f = 0; f < m_num_features; f++)
			{
				features[f] = m_common_features[f];
			}
		}

		if (m_other_context != nullptr)
		{
			other_context = new std::string(m_other_context->c_str());
		}

		return new Context(features, m_num_features, other_context);
	}

	void Serialize(std::ostringstream& stream)
	{
		if (m_common_features != nullptr)
		{
			for (size_t i = 0; i < m_num_features; i++)
			{
				stream << m_common_features[i].Index << ":" << m_common_features[i].X << " ";
			}
		}
		/* TODO: How should we deal with other context?
		if (m_other_context != nullptr)
		{
			stream << "| " << *m_other_context;
		}
		*/
	}

	void Get_Features(MWTFeature*& features, size_t& num_features)
	{
		features = m_common_features;
		num_features = m_num_features;
	}

	void Get_Other_Context(std::string*& other_context)
	{
		m_other_context = other_context;
	}

private:
	MWTFeature* m_common_features;
	size_t m_num_features;
	std::string* m_other_context;
	std::string m_external_other_context;
	bool m_is_copy;
};

class Interaction : public Serializable
{
public:
	Interaction(Context* context, MWTAction action, float prob, std::string unique_id, bool is_copy = false) : 
  m_context(context), m_action(action), m_prob(prob), m_id(unique_id), m_is_copy(is_copy)
	{
		m_reward = NO_REWARD;
		m_id_hash = HashUtils::Compute_Id_Hash(unique_id);
		// By default, assume the external context is the same as the one passed in above, but 
		// (C#) interop to work the external context should be set to a managed pointer
		m_external_context = context;
	}

	~Interaction()
	{
		if (m_is_copy)
		{
			delete m_context;
		}
	}

	std::string Get_Id()
	{
		return m_id;
	}

	u64 Get_Id_Hash()
	{
		return m_id_hash;
	}

	MWTAction Get_Action()
	{
		return m_action;
	}

	float Get_Prob()
	{
		return m_prob;
	}

	Context* Get_Context()
	{
		return m_context;
	}

	float Get_Reward()
	{
		return m_reward;
	}

	void Set_Reward(float reward)
	{
		m_reward = reward;
	}

	Interaction* Copy()
	{
		return new Interaction(m_context->Copy(), m_action, m_prob, m_id, true);
	}

	void Serialize(std::ostringstream& stream)
	{
		m_action.Serialize(stream);
		// Use 2 decimal places for reward, probability
		stream << " " << m_id << " ";
		stream << std::setprecision(5) << m_prob << " | "; 
		m_context->Serialize(stream);
	}

	void Serialize_VW_CSOAA(std::ostringstream& stream)
	{
		// Format is [action]:[cost]:[probability] | [features]
		m_action.Serialize(stream);
		// The cost is the importance-weighted reward, negated because the learner minimizes the cost
		stream << ":" << std::fixed << std::setprecision(5) << -m_reward << ":" << m_prob << " | ";
		m_context->Serialize(stream);
	}

// Required for (C#) interop
public:
	void* Get_External_Context()
	{
		return m_external_context;
	}

	void Set_External_Context(void* ext_context)
	{
		m_external_context = ext_context;
	}
	
private:
	Context* m_context;
	MWTAction m_action;
	float m_prob;
	float m_reward;
	std::string m_id;
	u64 m_id_hash;
	bool m_is_copy;

	// Required for (C#) interop
	void* m_external_context;
};

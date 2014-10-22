//
// Classes and definitions for interacting with the MWT service.
//
#pragma once

#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <float.h>
#include "Common.h" 

using namespace std;

//TODO: Make this a static const float of the Interaction class (right now VS doesn't support
// the C++11 constexpr keyword, which is needed to initialize such non-integer types.
#define NO_REWARD -FLT_MAX

namespace MultiWorldTesting {

class Serializable
{
public:
	virtual void Serialize(std::string&) = 0;
};

struct Feature
{
	float Value;
	u32 Id;

	bool operator==(Feature other_feature)
	{
		return Id == other_feature.Id;
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

	bool Match(MWTAction& second_action)
	{
		return m_id == second_action.Get_Id();
	}

	void Serialize(std::string& stream)
	{
		stream.append(to_string(m_id));
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
	Context(Feature* common_features, size_t num_features) : 
		m_common_features(common_features), 
		m_num_features(num_features),
		m_other_context(""),
		m_is_copy(false)
	{
	}

	Context(Feature* common_features, size_t num_features, std::string other_context) :
		m_common_features(common_features), 
		m_num_features(num_features), 
		m_other_context(other_context),
		m_is_copy(false)
	{
	}

	~Context()
	{
		if (m_is_copy)
		{
			delete[] m_common_features;
		}
	}

	void Serialize(std::string& stream)
	{
		if (m_common_features != nullptr)
		{
			for (size_t i = 0; i < m_num_features; i++)
			{
				stream.append(to_string(m_common_features[i].Id));
				stream.append(":", 1);

				char feature_str[15] = { 0 };
				NumberUtils::Float_To_String(m_common_features[i].Value, feature_str);

				stream.append(feature_str);
				if (i < m_num_features - 1)
				{
					stream.append(" ", 1);
				}
			}
		}
	}

	void Get_Features(Feature*& features, size_t& num_features)
	{
		features = m_common_features;
		num_features = m_num_features;
	}

	void Get_Other_Context(std::string& other_context)
	{
		other_context = m_other_context;
	}

private:
	Context(Feature* common_features, size_t num_features,
		std::string other_context, bool is_copy) :
		m_common_features(common_features),
		m_num_features(num_features),
		m_other_context(other_context),
		m_is_copy(is_copy)
	{
	}

	Context* Copy()
	{
		Feature* features = nullptr;

		if (m_num_features > 0 && m_common_features != nullptr)
		{
			features = new Feature[m_num_features];
			memcpy(features, m_common_features, sizeof(Feature)*m_num_features);
		}

		return new Context(features, m_num_features, m_other_context, true);
	}

private:
	Feature* m_common_features;
	size_t m_num_features;
	std::string m_other_context;
	std::string m_external_other_context;
	bool m_is_copy;

	friend class Interaction;
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

	void Serialize(std::string& stream)
	{
		m_action.Serialize(stream);
		// Use 2 decimal places for reward, probability
		stream.append(" ", 1);
		stream.append(m_id);
		stream.append(" ", 1);
		
		char prob_str[10] = { 0 };
		NumberUtils::Float_To_String(m_prob, prob_str);
		stream.append(prob_str);

		stream.append(" | ", 3);
		m_context->Serialize(stream);
	}

	void Serialize_VW(std::string& stream)
	{
		// Format is [action]:[cost]:[probability] | [features]
		m_action.Serialize(stream);
		// The cost is the importance-weighted reward, negated because the learner minimizes the cost
		stream.append(":");
		stream.append(to_string(-m_reward));
		stream.append(":");
		stream.append(to_string(m_prob));
		stream.append(" | ");
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
	Interaction* Copy()
	{
		return new Interaction(m_context->Copy(), m_action, m_prob, m_id, true);
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

	friend class InteractionStore;
};

class InteractionStore
{
private:

	void Clear_Data()
	{
		for (size_t i = 0; i < m_interactions.size(); i++)
			delete m_interactions[i];
		m_interactions.clear();
	}

public:

	InteractionStore() { this->Clear_Data(); }

	~InteractionStore()
	{ // If the store is deleted while still having in-mem data then try flushing it
		if (m_interactions.size() > 0)
			throw std::invalid_argument("MWT still has data during destruction");
	}

	void Store(Interaction* interaction)
	{
		if (interaction == nullptr)
			throw std::invalid_argument("Interaction to store is NULL");
		m_interactions.push_back(interaction->Copy());
	}

	void Store(std::vector<Interaction*> interactions)
	{
		if (interactions.size() == 0)
			throw std::invalid_argument("Interaction set to store is empty");
		for (size_t i = 0; i < interactions.size(); i++)
			this->Store(interactions[i]);
	}

	std::string Get_All_Interactions()
	{
		std::string serialized_string;
		if (m_interactions.size() > 0)
		{
			serialized_string.reserve(1000);

			for (size_t i = 0; i < m_interactions.size(); i++)
			{
				m_interactions[i]->Serialize(serialized_string);
				if (i < m_interactions.size() - 1)
				{
					serialized_string.append("\n");
				}
			}

			this->Clear_Data();
		}
		return serialized_string;
	}

	void Get_All_Interactions(size_t& num_interactions, Interaction**& interactions)
	{
		if (m_interactions.size() > 0)
		{
			num_interactions = m_interactions.size();
			interactions = new Interaction*[num_interactions];
			for (size_t i = 0; i < m_interactions.size(); i++)
				interactions[i] = m_interactions[i];
			m_interactions.clear();
		}
		else
		{
			num_interactions = 0;
			interactions = nullptr;
		}
	}

private:
	std::vector<Interaction*> m_interactions;
};
}

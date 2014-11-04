//
// Classes and definitions for interacting with the MWT service.
//
#pragma once

#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <float.h>
#include <math.h>
#include "Common.h" 

using namespace std;

//TODO: Make this a static const float of the Interaction class (right now VS doesn't support
// the C++11 constexpr keyword, which is needed to initialize such non-integer types.
#define NO_REWARD -FLT_MAX

MWT_NAMESPACE {

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

class BaseContext : Serializable
{
private:
	virtual void Get_Features(size_t& num_features, Feature*& features) = 0;

	void Serialize(std::string& stream)// final // does not compile in g++
	{
		Feature* features = nullptr;
		size_t num_features;
		Get_Features(num_features, features);
		if (features != nullptr && num_features > 0)
		{
			char feature_str[35] = { 0 };

			for (size_t i = 0; i < num_features; i++)
			{
				int chars;
				if (i == 0)
				{
					chars = sprintf(feature_str, "%d:", features[i].Id);
				}
				else
				{
					chars = sprintf(feature_str, " %d:", features[i].Id);
				}
				NumberUtils::print_float(feature_str + chars, features[i].Value);

				stream.append(feature_str);
			}
		}
	}

	friend class Interaction;
};

class SimpleContext : public BaseContext
{
public:
	SimpleContext(Feature* common_features, size_t num_features) :
		m_common_features(common_features), 
		m_num_features(num_features),
		m_other_context(""),
		m_is_copy(false)
	{
	}

	SimpleContext(Feature* common_features, size_t num_features, std::string other_context) :
		m_common_features(common_features), 
		m_num_features(num_features), 
		m_other_context(other_context),
		m_is_copy(false)
	{
	}

	~SimpleContext()
	{
		if (m_is_copy)
		{
			delete[] m_common_features;
		}
	}

	void Get_Features(size_t& num_features, Feature*& features)
	{
		features = m_common_features;
		num_features = m_num_features;
	}

	void Get_Other_Context(std::string& other_context)
	{
		other_context = m_other_context;
	}

private:
	//TODO: Either remove this or move it to a copy constructor that we use when storing an Interaction
	void Set_Is_Copy(bool is_copy)
	{
		m_is_copy = is_copy;
	}

	SimpleContext* Copy()
	{
		Feature* features = nullptr;

		if (m_num_features > 0 && m_common_features != nullptr)
		{
			features = new Feature[m_num_features];
			memcpy(features, m_common_features, sizeof(Feature)*m_num_features);
		}

		SimpleContext* context = new SimpleContext(features, m_num_features, m_other_context);
		context->Set_Is_Copy(true);
		return context;
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
	Interaction(BaseContext* context, MWTAction action, float prob, std::string unique_id, bool is_copy = false) : 
  m_context(context), m_action(action), m_prob(prob), m_id(unique_id), m_is_copy(is_copy)
	{
		m_reward = NO_REWARD;
		m_id_hash = HashUtils::Compute_Id_Hash(unique_id);
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

	BaseContext* Get_Context()
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

private:
	Interaction* Copy()
	{
		//TODO: Set is_copy to false since we are no longer creating a copy of the context
		return new Interaction(m_context, m_action, m_prob, m_id, false);
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

PORTING_INTERFACE:
	void* Get_Clr_Context()
	{
		return m_external_context;
	}

	void Set_Clr_Context(void* ext_context)
	{
		m_external_context = ext_context;
	}

#ifdef TEST_CPP
public:
#else
private:
#endif
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

private:
	BaseContext* m_context;
	MWTAction m_action;
	float m_prob;
	float m_reward;
	std::string m_id;
	u64 m_id_hash;
	bool m_is_copy;

	// Required for (C#) interop
	void* m_external_context;

	friend class InteractionStore;
	friend class MWTRewardReporter;
	friend class MWTOptimizer;
};

class InteractionStore
{
private:

	void Clear_Data()
	{
		m_interactions.clear();
	}

public:

	InteractionStore() { this->Clear_Data(); }

	~InteractionStore()
	{ // If the store is deleted while still having in-mem data then try flushing it
		if (m_interactions.size() > 0)
			throw std::invalid_argument("MWT still has data during destruction");
	}

	void Store(Interaction& interaction)
	{
		m_interactions.push_back(interaction);
	}

	std::string Get_All_Interactions_As_String()
	{
		std::string serialized_string;
		if (m_interactions.size() > 0)
		{
			serialized_string.reserve(1000);

			for (size_t i = 0; i < m_interactions.size(); i++)
			{
				m_interactions[i].Serialize(serialized_string);
				if (i < m_interactions.size() - 1)
				{
					serialized_string.append("\n");
				}
			}

			this->Clear_Data();
		}
		return serialized_string;
	}

	std::vector<Interaction> Get_All_Interactions()
	{
		std::vector<Interaction> interactions = m_interactions;

		this->Clear_Data();

		return interactions;
	}

private:
	std::vector<Interaction> m_interactions;
};
}

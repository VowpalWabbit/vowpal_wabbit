//
// Classes and definitions for interacting with the MWT service.
//
#pragma once

#include "stdafx.h"
#include "example.h"
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

#define NO_JOIN_KEY -99999

class Serializable
{
public:
	virtual void Serialize(std::ostringstream&) = 0;
};

class IdGenerator
{
public:
	static void Initialize()
	{
		g_id = 0;
		::InitializeCriticalSection(&g_id_mutex);
	}

	static u64 Get_Id()
	{
		::EnterCriticalSection(&g_id_mutex);
		u64 return_id = g_id;
		g_id++;
		::LeaveCriticalSection(&g_id_mutex);

		return return_id;
	}

	static void Destroy()
	{
		::DeleteCriticalSection(&g_id_mutex);
	}

private:
	static u64 g_id;
	static CRITICAL_SECTION g_id_mutex;
};

class MWTAction : public Serializable
{
public:
	MWTAction(u32 id) : m_id(id)
	{
	}
	u32 Get_Id() const { return m_id; }
	u32 Get_Id_ZeroBased() const { return m_id - 1; }

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
	ActionSet(u32 count) : m_count(count)
	{
		for (u32 i = 0; i < count; i++)
		{
			m_action_set.push_back(MWTAction(i + 1)); // 1-based Action id
		}
	}

	~ActionSet()
	{
	}

	MWTAction Get(u32 id)
	{
		return m_action_set.at(id);
	}

	u32 Count()
	{
		return m_count;
	}

	virtual bool Match(MWTAction* firstAction, MWTAction* secondAction)
	{
		return firstAction->Get_Id() == secondAction->Get_Id();
	}

private:
	std::vector<MWTAction> m_action_set;
	int m_count;
};

class Context : public Serializable
{
public:
	Context(feature* common_features, size_t num_features, bool is_copy = false) : 
		m_common_features(common_features), 
		m_num_features(num_features),
		m_other_context(nullptr),
		m_is_copy(is_copy)
	{
	}

	Context(feature* common_features, size_t num_features, 
		std::string* other_context, bool is_copy = false) :
		m_common_features(common_features), 
		m_num_features(num_features), 
		m_other_context(other_context),
		m_is_copy(is_copy)
	{
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
		feature* features = nullptr;
		std::string* other_context = nullptr;

		if (m_num_features > 0 && m_common_features != nullptr)
		{
			features = new feature[m_num_features];
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
				stream << m_common_features[i].weight_index << ":" << m_common_features[i].x << " ";
			}
		}
		if (m_other_context != nullptr)
		{
			stream << "| " << *m_other_context;
		}
	}

private:
	feature* m_common_features;
	size_t m_num_features;
	std::string* m_other_context;
	bool m_is_copy;
};

class Policy
{
public:
	virtual std::tuple<MWTAction, float, bool> Choose_Action(void* context, ActionSet& actions) = 0;
	virtual std::tuple<MWTAction, float, bool> Choose_Action(void* context, ActionSet& actions, u32 seed) = 0;
	virtual ~Policy()
	{
	}
};

class Interaction : public Serializable
{
public:
	Interaction(Context* context, MWTAction action, float prob, u64 unique_id = 0, bool is_copy = false) : 
		m_context(context), m_action(action), m_prob(prob), m_is_copy(is_copy)
	{
		if (unique_id > 0)
		{
			m_id = unique_id;
		}
		else
		{
			m_id = IdGenerator::Get_Id();
		}
	}

	~Interaction()
	{
		if (m_is_copy)
		{
			delete m_context;
		}
	}

	u64 Get_Id()
	{
		return m_id;
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

	Interaction* Copy()
	{
		return new Interaction(m_context->Copy(), m_action, m_prob, m_id, true);
	}

	void Serialize(std::ostringstream& stream)
	{
		m_action.Serialize(stream);
		stream << ":0:"; // for now report reward as 0
		stream << std::fixed << std::setprecision(2) << m_prob << " | "; // 2 decimal places probability
		m_context->Serialize(stream);
	}

private:
	Context* m_context;
	MWTAction m_action;
	float m_prob;
	u64 m_id;
	bool m_is_copy;
};
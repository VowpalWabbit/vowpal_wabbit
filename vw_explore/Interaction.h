//
// Classes and definitions for interacting with the MWT service.
//

#include "stdafx.h"
#include "example.h"
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

class Serializable
{
public:
	virtual void Serialize(std::ostringstream&) = 0;
};

class Id_Generator
{
public:
	static void Initialize()
	{
		g_id = 0;
	}

	static u64 Get_Id()
	{
		return g_id++;
	}

private:
	static std::atomic_uint64_t g_id;
};

class Action : public Serializable
{
public:
	Action(u32 id) : m_id(id)
	{
	}
	u32 Get_Id() const { return m_id; }

	void Serialize(std::ostringstream& stream)
	{
		stream << m_id;
	}

private:
	u32 m_id;
};

class Action_Set
{
public:

	// TODO: support opaque IDs, will need to update Action class as well
	// e.g. add GetStringID() or GetDescription() etc...
	Action_Set(u32 count) : m_count(count)
	{
		for (u32 i = 0; i < count; i++)
		{
			m_action_set.push_back(Action(i + 1)); // 1-based Action id
		}
	}

	~Action_Set()
	{
	}

	Action Get(u32 id)
	{
		return m_action_set.at(id);
	}

	u32 Count()
	{
		return m_count;
	}

	// TODO: should support GetAction() methods with a few overloads. current there's no way to iterate or get an action out of the set

	virtual bool Match(Action* firstAction, Action* secondAction)
	{
		return firstAction->Get_Id() == secondAction->Get_Id();
	}

private:
	std::vector<Action> m_action_set;
	int m_count;
};

class Context : public Serializable
{
public:
	Context(std::vector<feature> common_features) : m_common_features(common_features)
	{
	}

	Context(std::vector<feature> common_features, std::string other_context) :
		m_common_features(common_features), m_other_context(other_context)
	{
	}

	virtual bool Is_Match(Context* second_context)
	{
		return (m_common_features.size() == second_context->m_common_features.size() &&
			std::equal(m_common_features.begin(), m_common_features.end(), second_context->m_common_features.begin()));
	}

	void Serialize(std::ostringstream& stream)
	{
		for (size_t i = 0; i < m_common_features.size(); i++)
		{
			stream << m_common_features[i].weight_index << ":" << m_common_features[i].x << " ";
		}
		stream << "| " << m_other_context;
	}

private:
	std::vector<feature> m_common_features;
	std::string m_other_context;
};

class Policy
{
public:
	virtual std::pair<Action, float> Choose_Action(Context& context, Action_Set& actions) = 0;
	virtual std::pair<Action, float> Choose_Action(Context& context, Action_Set& actions, u32 seed) = 0;
	virtual ~Policy()
	{
	}
};

class Interaction : public Serializable
{
public:
	Interaction(Context* context, Action action, float prob, u64 unique_id = 0) : 
		m_context(context), m_action(action), m_prob(prob)
	{
		if (unique_id > 0)
		{
			m_id = unique_id;
		}
		else
		{
			m_id = Id_Generator::Get_Id();
		}
	}

	u64 Get_Id()
	{
		return m_id;
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
	Action m_action;
	float m_prob;
	u64 m_id;
};
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
}

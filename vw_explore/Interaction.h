//
// Classes and definitions for interacting with the MWT service.
//

#include "stdafx.h"
#include "example.h"

class Serializable
{
public:
	virtual ~Serializable(){}
	virtual void Serialize(std::stringstream&) = 0;
};

class Id_Generator
{
public:
	static void Initialize()
	{
		gId = 0;
	}

	static u64 Get_Id()
	{
		return gId++;
	}

private:
	static std::atomic_uint64_t gId;
};

class Action : public Serializable
{
public:
	Action(u32 id) : id(id)
	{
	}
	u32 Get_Id() const { return id; }

	void Serialize(std::stringstream& stream)
	{
		//TODO: implement
	}

private:
	u32 id;
};

class Action_Set
{
public:

	// TODO: support opaque IDs, will need to update Action class as well
	// e.g. add GetStringID() or GetDescription() etc...
	Action_Set(u32 count) : count(count)
	{
		for (u32 i = 0; i < count; i++)
		{
			actionSet.push_back(Action(i + 1)); // 1-based Action id
		}
	}

	~Action_Set()
	{
	}

	Action Get(u32 id)
	{
		return actionSet.at(id);
	}

	u32 Count()
	{
		return count;
	}

	// TODO: should support GetAction() methods with a few overloads. current there's no way to iterate or get an action out of the set

	virtual bool Match(Action* firstAction, Action* secondAction)
	{
		return firstAction->Get_Id() == secondAction->Get_Id();
	}

private:
	std::vector<Action> actionSet;
	int count;
};

class Context : public Serializable
{
public:
	Context(std::vector<feature> commonFeatures) : commonFeatures(commonFeatures)
	{
	}

	Context(std::vector<feature> commonFeatures, std::string otherContext) : 
		commonFeatures(commonFeatures), otherContext(otherContext)
	{
	}

	virtual bool Is_Match(Context* secondContext)
	{
		return (commonFeatures.size() == secondContext->commonFeatures.size() &&
			std::equal(commonFeatures.begin(), commonFeatures.end(), secondContext->commonFeatures.begin()));
	}

	void Serialize(std::stringstream& stream)
	{
		//TODO: implement
	}

private:
	std::vector<feature> commonFeatures;
	std::string otherContext;
};

class Policy
{
public:
	virtual std::pair<Action, float> Choose_Action(Context& context, Action_Set& actions) = 0;
	virtual ~Policy()
	{
	}
};

class Interaction : public Serializable
{
public:
	Interaction(Context* context, Action action, float prob, bool isCopy = false) : 
		pContext(context), action(action), prob(prob)
	{
		id = Id_Generator::Get_Id();
	}

	u64 Get_Id()
	{
		return id;
	}

	void Serialize(std::stringstream& stream)
	{
		pContext->Serialize(stream);
		action.Serialize(stream);
	}

private:
	Context* pContext;
	Action action;
	float prob;
	u64 id;
};
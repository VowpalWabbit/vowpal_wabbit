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
			actionSet.push_back(Action(i));
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
	std::vector<Action> actionSet; // TODO: should be 1-based
	int count;
};

class Context : public Serializable
{
public:
	/*
	Context() : commonFeature()
	{
		commonFeature = new std::vector<Feature>();
		actionFeatures = new std::map<Action, Feature>();
	}
	*/

	Context(std::vector<feature> commonFeatures, std::map<Action, std::vector<feature>> actionFeatures) :
		commonFeatures(commonFeatures), actionFeatures(actionFeatures)
	{
	}

	Context(std::vector<feature> commonFeatures) : commonFeatures(commonFeatures)
	{
	}

	virtual bool Is_Match(Context* secondContext)
	{
		// Compare common features
		bool match = (
			commonFeatures.size() == secondContext->commonFeatures.size() &&
			std::equal(commonFeatures.begin(), commonFeatures.end(), secondContext->commonFeatures.begin())
		);
		match &= (actionFeatures.size() == secondContext->actionFeatures.size());
		
		if (match)
		{
			// TODO: implement
			/*std::map<Action, feature>::iterator thisIter = actionFeatures.begin();
			std::map<Action, feature>::iterator thatIter = secondContext->actionFeatures.begin();
			while (thisIter != actionFeatures.end())
			{
				if (!(thisIter->first.Get_Id() == thatIter->first.Get_Id() && 
					thisIter->second == thatIter->second))
				{
					match = false;
					break;
				}
				thisIter++;
				thatIter++;
			}*/
			// TODO: should this also compare other context? Probably not.
		}
		
		return match;
	}

	void Serialize(std::stringstream& stream)
	{
		//TODO: implement
	}

private:
	std::vector<feature> commonFeatures;
	std::map<Action, std::vector<feature>> actionFeatures;
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
	Interaction(Context* context, Action action, double prob, bool isCopy = false) : 
		pContext(context), action(action), prob(prob), isCopy(isCopy)
	{
		id = Id_Generator::Get_Id();
	}

	~Interaction()
	{
		if (isCopy)
		{
			delete pContext;
		}
	}

	u64 Get_Id()
	{
		return id;
	}

	Interaction* Copy()
	{
		return new Interaction(new Context(*pContext), action, prob, /* isCopy = */ true);
	}

	void Serialize(std::stringstream& stream)
	{
		pContext->Serialize(stream);
		action.Serialize(stream);
	}

private:
	Context* pContext;
	Action action;
	double prob;
	bool isCopy;
	u64 id;
};
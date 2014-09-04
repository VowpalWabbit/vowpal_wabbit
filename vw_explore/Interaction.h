//
// Classes and definitions for interacting with the MWT service.
//

#include "stdafx.h"
#include "example.h"

class ISerializable
{
public:
	virtual ~ISerializable(){}
	virtual void Serialize(u8*&, int&) = 0;
	virtual void Deserialize(u8*, int) = 0;
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

class Action : public ISerializable
{
public:
	Action(u32 id) : id(id)
	{
	}
	u32 Get_Id() const { return id; }

	void Serialize(u8*& data, int& length)
	{
		//TODO: implement
	}

	void Deserialize(u8* data, int length)
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
	Action_Set(u32 count)
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
		return (u32)actionSet.size();
	}

	// TODO: should support GetAction() methods with a few overloads. current there's no way to iterate or get an action out of the set

	virtual bool Match(Action* firstAction, Action* secondAction)
	{
		return firstAction->Get_Id() == secondAction->Get_Id();
	}

private:
	std::vector<Action> actionSet;
};

class Context : public ISerializable
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

	void Serialize(u8*& data, int& length)
	{
		//TODO: implement
	}

	void Deserialize(u8* data, int length)
	{
		//TODO: implement
	}

private:
	std::vector<feature> commonFeatures;
	std::map<Action, std::vector<feature>> actionFeatures;
	std::string otherContext;
};

class Reward : public ISerializable
{
public:
	Reward(double reward) : reward(reward)
	{
	}

	Reward(double reward, std::string otherOutcomes) : reward(reward), otherOutcomes(otherOutcomes)
	{
	}

	double Get()
	{
		return reward;
	}

	void Serialize(u8*& data, int& length)
	{
		//TODO: implement
	}

	void Deserialize(u8* data, int length)
	{
		//TODO: implement
	}

private:
	double reward;
	std::string otherOutcomes;
};

class Policy
{
public:
	virtual std::pair<Action, float> Choose_Action(Context& context, Action_Set& actions) = 0;
	virtual ~Policy()
	{
	}
};

class Interaction : public ISerializable
{
public:
	Interaction(Context* context, Action action, double prob, bool isCopy = false) : 
		pContext(context), action(action), prob(prob), isCopy(isCopy)
	{
		pReward = nullptr;
		id = Id_Generator::Get_Id();
	}

	~Interaction()
	{
		if (isCopy)
		{
			delete pContext;
		}
	}

	void Update_Reward(Reward* reward)
	{
		pReward = reward;
	}

	u64 Get_Id()
	{
		return id;
	}

	Interaction* Copy()
	{
		return new Interaction(new Context(*pContext), action, prob, /* isCopy = */ true);
	}

	/*
	public override string ToString()
	{
	string outString = "||i t:" + Context.Timestamp + " p:" + PScore + " tag:" + Tag + " ||a 1:" + Action.Arm + " ||r 1:" + Feedback.GetReward()
	+ " ||x " + Context.CommonFeature.ToString();
	foreach(KeyValuePair<string, Feature> entry in Context.ArmFeatures)
	outString += " ||" + entry.Key + " " + entry.Value.ToString();
	return outString;
	}
	*/

	void Serialize(u8*& data, int& length)
	{
		// TODO: Count length of data in bytes
		length = 0;

		pContext->Serialize(data, length);
		action.Serialize(data, length);

		if (pReward != nullptr)
		{
			pReward->Serialize(data, length);
		}

	}

	void Deserialize(u8* data, int length)
	{
		pContext->Deserialize(data, length);
		action.Deserialize(data, length);
		pReward->Deserialize(data, length);
	}

private:
	Context* pContext;
	Action action;
	Reward* pReward;
	double prob;
	bool isCopy;
	u64 id;
};
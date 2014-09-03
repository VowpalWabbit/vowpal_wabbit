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

class IDGenerator
{
public:
	static void Initialize()
	{
		gId = 0;
	}

	static u64 GetID()
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
	u32 GetID() const { return id; }

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

class ActionSet
{
public:

	ActionSet(u32 startId, u32 endId)
	{
		startAction = new Action(startId);
		endAction = new Action(endId);
	}

	ActionSet(std::vector<Action*> actionSet) : actionSet(actionSet)
	{
	}

	~ActionSet()
	{
		delete startAction;
		delete endAction;
	}

	// TODO: should support GetAction() methods with a few overloads. current there's no way to iterate or get an action out of the set

	virtual bool Match(Action* firstAction, Action* secondAction)
	{
		return firstAction->GetID() == secondAction->GetID();
	}

private:
	Action* startAction;
	Action* endAction;
	std::vector<Action*> actionSet;
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

	Context(std::vector<feature> commonFeatures, std::map<Action, feature> actionFeatures) :
		commonFeatures(commonFeatures), actionFeatures(actionFeatures)
	{
	}

	Context(std::vector<feature> commonFeatures) : commonFeatures(commonFeatures)
	{
	}

	virtual bool IsMatch(Context* secondContext)
	{
		// Compare common features
		bool match = (
			commonFeatures.size() == secondContext->commonFeatures.size() &&
			std::equal(commonFeatures.begin(), commonFeatures.end(), secondContext->commonFeatures.begin())
		);
		match &= (actionFeatures.size() == secondContext->actionFeatures.size());
		
		if (match)
		{
			std::map<Action, feature>::iterator thisIter = actionFeatures.begin();
			std::map<Action, feature>::iterator thatIter = secondContext->actionFeatures.begin();
			while (thisIter != actionFeatures.end())
			{
				if (!(thisIter->first.GetID() == thatIter->first.GetID() && 
					thisIter->second == thatIter->second))
				{
					match = false;
					break;
				}
				thisIter++;
				thatIter++;
			}
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
	std::map<Action, feature> actionFeatures;
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
	virtual std::pair<Action, float> ChooseAction(Context& context, ActionSet& actions) = 0;
	virtual ~Policy()
	{
	}
};

class Interaction : public ISerializable
{
public:
	Interaction(Context& context, Action action, double prob) : rContext(context), action(action), prob(prob)
	{
		pReward = nullptr;
		id = IDGenerator::GetID();
	}

	void UpdateReward(Reward* reward)
	{
		pReward = reward;
	}

	u64 GetId()
	{
		return id;
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

		rContext.Serialize(data, length);
		action.Serialize(data, length);

		if (pReward != nullptr)
		{
			pReward->Serialize(data, length);
		}

	}

	void Deserialize(u8* data, int length)
	{
		rContext.Deserialize(data, length);
		action.Deserialize(data, length);
		pReward->Deserialize(data, length);
	}

private:
	Context& rContext;
	Action action;
	Reward* pReward;
	double prob;
	u64 id;
};
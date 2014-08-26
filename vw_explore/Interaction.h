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

class Interaction : public ISerializable
{
public:
	Interaction(Context* context, Action* action, double prob) : pContext(context), pAction(action), prob(prob)
	{
		pReward = nullptr;
		id = gId++;
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
		if (pContext == nullptr ||
			pAction == nullptr)
		{
			throw;
		}

		// TODO: Count length of data in bytes
		length = 0;

		pContext->Serialize(data, length);
		pAction->Serialize(data, length);

		if (pReward == nullptr)
		{
			pReward->Serialize(data, length);
		}

	}

	void Deserialize(u8* data, int length)
	{
		pContext->Deserialize(data, length);
		pAction->Deserialize(data, length);
		pReward->Deserialize(data, length);
	}

private:
	Context* pContext = nullptr;
	Action* pAction = nullptr;
	Reward* pReward = nullptr;
	double prob = 0.0;
	u64 id;

	static std::atomic_uint64_t gId = 0;
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

	virtual bool IsMatch(Context secondContext)
	{
		return (commonFeatures == secondContext.commonFeatures) && (actionFeatures == secondContext.actionFeatures);
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

private:
	double reward;
	std::string otherOutcomes;
};

class Action : public ISerializable
{
public:
	Action(u32 id) : id(id)
	{
	}	

	u32 id;
};

class ActionSet
{
public:

	ActionSet(u32 startId, u32 endId) : startAction(startId), endAction(endId)
	{
	}

	ActionSet(std::vector<Action> actionSet) : actionSet(actionSet)
	{
	}

	virtual bool Match(Action firstAction, Action secondAction)
	{
		return firstAction == secondAction;
	}

private:
	Action startAction;
	Action endAction;
	std::vector<Action> actionSet;
};

class Policy
{
	virtual std::pair<Action, float> ChooseAction(Context* context, ActionSet* actions) = 0;
	virtual ~Policy()
	{
	}
};
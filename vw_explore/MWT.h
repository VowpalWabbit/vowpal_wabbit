//
// Main interface for clients to the MWT service.
//

#include "stdafx.h"

class Explorer : public Policy
{
public:
	virtual void AdjustFrequency(float frequency) = 0;
	virtual void StopExplore() = 0;
	virtual void StartExplore() = 0;
};

class EpsilonGreedyExplorer : public Explorer
{
public:
	EpsilonGreedyExplorer(float epsilon, Policy* defaultPolicy, bool smartExploration) : 
		epsilon(epsilon), defaultPolicy(defaultPolicy), smartExploration(smartExploration), doExplore(true)
	{
		if (defaultPolicy == nullptr)
		{
			throw std::invalid_argument("Default Policy must be specified.");
		}
		if (epsilon <= 0)
		{
			throw std::invalid_argument("Initial epsilon value must be positive.");
		}
	}

	std::pair<Action*, float> ChooseAction(Context* context, ActionSet* actions)
	{
		if (doExplore)
		{
			// Interface with VW
			// TODO: Samples uniformly or with learner during epsilon of the time
		}
		else
		{
			return defaultPolicy->ChooseAction(context, actions);
		}
	}

	void AdjustFrequency(float frequency)
	{
		epsilon += frequency;
	}

	void StopExplore()
	{
		doExplore = false;
	}
	
	void StartExplore()
	{
		doExplore = true;
	}

private:
	float epsilon;
	Policy* defaultPolicy;
	bool smartExploration;
	bool doExplore;
};

class MWT
{
	MWT()
	{
		IDGenerator::Initialize();

		// TODO: where does appId come from?
		pLogger = new Logger(appId);
	}

	~MWT()
	{
		delete pLogger;
		delete pExplorer;
	}

	void InitializeEpsilonGreedy(float epsilon, Policy* defaultPolicy, float explorationBudget, bool smartExploration = false)
	{
		pExplorer = new EpsilonGreedyExplorer(epsilon, defaultPolicy, smartExploration);
	}

	std::pair<Action*, u64> ChooseAction(Context* context, ActionSet* actions)
	{
		auto actionProb = pExplorer->ChooseAction(context, actions);
		Interaction* pInteraction = new Interaction(context, actionProb.first, actionProb.second);
		pLogger->Store(pInteraction);
		
		// TODO: Anything else to do here?

		return std::pair<Action*, u64>(actionProb.first, pInteraction->GetId());
	}

	void ReportReward(u64 id, Reward* reward)
	{
		pLogger->Join(id, reward);
		// TODO: Update performance measures of current and default policy (estimated via offline eval)
		// TODO: Evaluate how we're doing relative to default policy 
	}


	/// <summary>
	/// Initializes learner with parameters specified in config.
	/// </summary>
	/// <param name="config"></param>
	/// <returns>Returns true if initialization succeeds, and false otherwise.</returns> 
	//bool Initialize(Config config);

	/// <summary>
	/// Prints out parameter and other information to a string for output for logging purposes.
	/// </summary>
	/// <returns></returns>
	//string Profile(string sep = "\n");

	/// <summary>
	/// Takes actions either in the learning flight (explore/exploit algAction) or in the deployment flight (exploit-only greedyAction).
	/// </summary>
	/// <param name="context"></param>
	/// <param name="algAction"></param>
	/// <param name="greedyAction"></param>
	//void ChooseAction(Context context, out BanditCommon.Action algAction, out BanditCommon.Action greedyAction);

	/// <summary>
	/// Updates internal policy with a new batch of interaction data.
	/// </summary>
	/// <param name="interactions"></param>
	//void UpdatePolicy(List<Interaction> interactions);

	/// <summary>
	/// Internal bookkeeping when an action (algAction in TakeAction) match occurs.
	/// </summary>
	//void NotifyMatch();

	/// <summary>
	/// Final logging information of the learner, to be called after learning finishes.
	/// </summary>
	/// <returns></returns>
	//string FinalLogInfo();

private:
	std::string appId;
	Explorer* pExplorer;
	Policy* defaultPolicy;
	Logger* pLogger;
};

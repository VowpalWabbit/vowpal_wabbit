//
// Main interface for clients to the MWT service.
//

#include "stdafx.h"

typedef Action PolicyFunc(void* data);

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
	EpsilonGreedyExplorer(float epsilon, PolicyFunc& defaultPolicyFunc) : 
		epsilon(epsilon), defaultPolicyFunc(defaultPolicyFunc), doExplore(true)
	{
		if (epsilon <= 0)
		{
			throw std::invalid_argument("Initial epsilon value must be positive.");
		}
	}

	std::pair<Action, float> ChooseAction(Context& context, ActionSet& actions)
	{
		if (doExplore)
		{
			// Interface with VW
			// TODO: Samples uniformly or with learner during epsilon of the time
			return std::pair<Action, float>(Action(0), 0.f);
		}
		else
		{
			// TODO: what is the argument type of default policy func?
			Action chosenAction = defaultPolicyFunc(nullptr);
			return std::pair<Action, float>(chosenAction, 1.f);
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
	PolicyFunc& defaultPolicyFunc;
	bool doExplore;
};

class MWT
{
public:
	MWT(std::string& appId)
	{
		IDGenerator::Initialize();

		if (appId.empty())
		{
			appId = this->GenerateAppId();
		}

		pLogger = new Logger(appId);
	}

	~MWT()
	{
		delete pLogger;
		delete pExplorer;
	}

	// TODO: should we restrict explorationBudget to some small numbers to prevent users from unwanted effect?
	//void InitializeEpsilonGreedy(float epsilon, Policy& defaultPolicy, float explorationBudget, bool smartExploration = false)
	//{
	//	pExplorer = new EpsilonGreedyExplorer(epsilon, defaultPolicy, smartExploration);
	//}

	template<class T>
	void InitializeEpsilonGreedy(float epsilon, PolicyFunc defaultPolicyFunc, float explorationBudget)
	{
		pExplorer = new EpsilonGreedyExplorer(epsilon, defaultPolicyFunc);
	}

	// TODO: should include defaultPolicy here? From users view, it's much more intuitive
	std::pair<Action, u64> ChooseAction(Context& context, ActionSet& actions)
	{
		auto actionProb = pExplorer->ChooseAction(context, actions);
		Interaction* pInteraction = new Interaction(context, actionProb.first, actionProb.second);
		pLogger->Store(pInteraction);
		
		// TODO: Anything else to do here?

		return std::pair<Action, u64>(actionProb.first, pInteraction->GetId());
	}

	void ReportReward(u64 id, Reward* reward)
	{
		pLogger->Join(id, reward);
		// TODO: Update performance measures of current and default policy (estimated via offline eval)
		// TODO: Evaluate how we're doing relative to default policy 
	}

private:
	std::string GenerateAppId()
	{
		return ""; // TODO: implement
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
